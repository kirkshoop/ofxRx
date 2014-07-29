//
//  ofxRx.h
//

#ifndef OFXRX_H
#define OFXRX_H

#include "ofMain.h"

//
// openframeworks uses some macros that are not disambiguated
//
#undef B0
#undef all_true
#undef check

#if RXCPP_VIEW_TRACE
#include "rxcpp/rx-trace.hpp"

struct ofxRxTrace : rxcpp::trace_noop
{
    // use valid to work around lifetime issue
    // (of exit will destruct view_trace (by running at_exit) and
    // then destruct ofApp)
    ~ofxRxTrace() {
        valid = false;
    }
    ofxRxTrace() : valid(true) {}
    
    struct marble
    {
        const char* method;
        unsigned long long start;
        unsigned long long end;
    };
    struct subscribed
    {
        rxcpp::trace_id id;
        std::string value_type;
        std::string status;
        std::deque<marble> marbles;
        std::vector<rxcpp::trace_id> from;
        std::vector<rxcpp::trace_id> to;
    };
    typedef std::map<rxcpp::trace_id, subscribed> subscribed_type;
    subscribed_type active;
    subscribed_type silent;
    bool valid;

    template<class Subscriber>
    inline void create_subscriber(const Subscriber& s) {
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        try{
            typename Subscriber::value_type* t = nullptr;
            active[key].value_type = typeid(*t).name();
            active[key].id = key;
        } catch(...) {
            active[key].value_type = "typeid unsupported.";
        }
        active[key].status = "created";
        s.add([=](){
            if(!valid) {return;}
            active[key].status = "disposed";
            silent[key] = std::move(active[key]);
            active.erase(key);
        });
    }

    template<class Observable, class Subscriber>
    inline void subscribe_enter(const Observable& o, const Subscriber& s) {
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "subscribed";
    }
    
    template<class OperatorSource, class OperatorChain, class Subscriber, class SubscriberLifted>
    inline void lift_enter(const OperatorSource&, const OperatorChain&, const Subscriber& s, const SubscriberLifted& sl) {
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        connect(sl, s);
        active[key].status = "lifted";
    }

    template<class SubscriberFrom, class SubscriberTo>
    inline void connect(const SubscriberFrom& from, const SubscriberTo& to) {
        auto fkey = from.get_id();
        if ((fkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto tkey = to.get_id();
        if ((tkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[fkey].to.push_back(tkey);
        active[tkey].from.push_back(fkey);
        active[tkey].status = "connected";
    }
    
    template<class SubscriptionState>
    inline void unsubscribe_enter(const SubscriptionState&) {
    }
    
    template<class Subscriber, class T>
    inline void on_next_enter(const Subscriber& s, const T&) {
        if(!valid) {return;}
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_next", now, now});
    }
    template<class Subscriber>
    inline void on_next_return(const Subscriber& s) {
        if(!valid) {return;}
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.back().end = now;
    }
    
    template<class Subscriber>
    inline void on_error_enter(const Subscriber& s, const std::exception_ptr&) {
        if(!valid) {return;}
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "errored";
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_error", now, now});
    }
    
    template<class Subscriber>
    inline void on_completed_enter(const Subscriber& s) {
        if(!valid) {return;}
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "completed";
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_completed", now, now});
    }
    
    void drawSubscribed(unsigned long long now, int height, ofColor glyphColor, ofColor textColor, const subscribed& s) {
        
        const int center = height/2;
        const int radius = height/2;
        const int lineHalfWidth = radius/4;
        const unsigned long long second = 1000000;
        const unsigned long long end = now;
        const unsigned long long begin = end - second;
        const unsigned long long size = end - begin;
        const int width = ofGetWidth();
        
        for (auto& m : s.marbles)
        {
            ofSetColor(glyphColor);
            ofSetLineWidth(lineHalfWidth*2);
            if (m.end < begin) continue;
            auto xs = ofMap(end-m.start, 0, size, width - radius, 0 + radius);
            auto xe = ofMap(end-m.end, 0, size, width - radius, 0 + radius);
            switch(m.method[3])
            {
                case 'n':
                    {
                        ofLine(xs - radius, center - lineHalfWidth, xe + radius, center - lineHalfWidth);
                    }
                    break;
                case 'e':
                    {
                        ofCircle(xs, center, radius / 1.5);
                        // slanted black line
                        ofSetColor(ofColor::black);
                        ofLine(xs - radius, height, xe + radius, 0);
                    }
                    break;
                case 'c':
                    {
                        // straight black line
                        ofSetColor(ofColor::black);
                        ofLine(xs + (center-lineHalfWidth), height, xs + (center-lineHalfWidth), 0);
                    }
                    break;
            }
        }
        
        ofSetColor(textColor);
        
        std::stringstream ss;
        ss << s.id << ": of " << s.value_type << " status: " << s.status << " -> marbles: " << s.marbles.size();
        if (s.marbles.size() > 0)
        {
            ss << ", from: " << s.marbles.front().start / second << "s, to: " << s.marbles.back().end / second << "s";
        }
        ss << " from[";
        for(auto& from : s.from) {
            ss << from << " ";
        }
        ss << "] to[";
        for(auto& to : s.to) {
            ss << to << " ";
        }
        ss << "]";
        
        ofDrawBitmapString(ss.str(), ofPoint(10, 14, 0));
        
    }
    
    void draw() {
        auto now = ofGetElapsedTimeMicros();
        int y = 0;
        const int height = 14;

        std::vector<const subscribed_type::value_type*> roots;
        std::transform(active.begin(), active.end(), std::back_inserter(roots),
            [&](const subscribed_type::value_type& v){
                return &v;
            });
        roots.erase(std::remove_if(roots.begin(), roots.end(),
            [](const subscribed_type::value_type* s){
                return !s->second.to.empty();
            }), roots.end());

        std::vector<std::deque<const subscribed_type::value_type*>> streams;
        std::function<void (const subscribed_type::value_type*, std::deque<const subscribed_type::value_type*>&)> fill_from;
        fill_from = [&](const subscribed_type::value_type* v, std::deque<const subscribed_type::value_type*>& stream) {
            for(auto& from : v->second.from) {
                auto next = active.find(from);
                if (next != active.end()) {
                    stream.push_front(&*next);
                }
            }
            for(auto& from : v->second.from) {
                auto next = active.find(from);
                if (next != active.end()) {
                    fill_from(&*next, stream);
                }
            }
        };
        std::transform(roots.begin(), roots.end(), std::back_inserter(streams),
                [&](const subscribed_type::value_type* v){
                    std::deque<const subscribed_type::value_type*> stream;
                    stream.push_front(v);
                    fill_from(v, stream);
                    return stream;
                });
/*
        sorted.erase(std::remove_if(sorted.begin(), sorted.end(),
            [](const subscribed_type::value_type* s){
                return false;//!s->second.to.empty();
            }), sorted.end());

        std::function<bool (const subscribed_type::value_type*, const subscribed_type::value_type*)> lhs_is_from;
        lhs_is_from = [&](const subscribed_type::value_type* lhs, const subscribed_type::value_type* rhs) {
            return lhs && rhs && std::find_if(lhs->second.to.begin(), lhs->second.to.end(),
                [&](const rxcpp::trace_id& cursor) {
                    auto next = active.find(cursor);
                    return cursor != rhs->first || lhs_is_from(&(*next), rhs);
                }) == lhs->second.to.end();
        };
        std::sort(sorted.begin(), sorted.end(), lhs_is_from);
*/
        for (auto stream : streams)
        {
            for (auto s : stream)
            {
                if (y > ofGetHeight()) {break;}
                
                ofPushMatrix();
                ofTranslate(0, y);
                ofFill();
                
                drawSubscribed(now, height, ofColor::green, ofColor::white, s->second);
                
                ofPopMatrix();
                
                y += (height + 1);
            }
            y += (height + 1);
        }
        for (auto& s : silent)
        {
            if (y > ofGetHeight()) {break;}
            
            ofPushMatrix();
            ofTranslate(0, y);
            ofFill();
            
            drawSubscribed(now, height, ofColor::red, ofColor::black, s.second);
            
            ofPopMatrix();
            
            y += (height + 1);
        }
    }
};

auto rxcpp_trace_activity(rxcpp::trace_tag) -> ofxRxTrace;
#endif

#include <rxcpp/rx.hpp>

namespace ofx {

namespace rx {

namespace rx=rxcpp;
namespace rxsc=rxcpp::rxsc;

}

}

namespace ofxRx = ofx::rx;

#include "ofxRxObservableFrom.h"
#include "ofxRxMouse.h"
#include "ofxRxKeyboard.h"
#include "ofxRxUpdates.h"

#endif
