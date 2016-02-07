//
//  ofxRx.h
//

#ifndef OFXRX_H
#define OFXRX_H

#include "ofMain.h"
#include "ofxGui.h"

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
    
    ofxPanel gui;
    ofxToggle show_stream_hud;
    ofxToggle show_silent;
    
    inline void setup()
    {
        ofAddListener(ofEvents().mousePressed,this,&ofxRxTrace::mousePressed);

        gui.setup();

        gui.add(show_stream_hud.setup("HUD", true));
        gui.add(show_silent.setup("inactive", false));
        
        gui.setPosition(ofGetWidth() - gui.getWidth() - 20, 20);
    }

    struct marble
    {
        const char* method;
        unsigned long long start;
        unsigned long long end;
    };
    struct subscribed
    {
        subscribed() : created(false), errored(false), completed(false), y(-1), show_from(false) {}
        rxcpp::trace_id id;
        std::string value_type;
        std::string status;
        bool created;
        bool errored;
        bool completed;
        int y;
        bool show_from;
        std::vector<marble> marbles;
        std::vector<rxcpp::trace_id> from;
        std::vector<rxcpp::trace_id> to;
    };
    typedef std::map<rxcpp::trace_id, subscribed> subscribed_type;
    subscribed_type active;
    subscribed_type silent;
    std::atomic<bool> valid;

    inline void mousePressed(ofMouseEventArgs & args) {
        auto clicked = std::find_if(active.begin(), active.end(),
            [&](const subscribed_type::value_type& s){
                return (s.second.y - 14) <= args.y && (s.second.y + 14 + 14) >= args.y;
            });
        if (clicked != active.end()) {
            clicked->second.show_from = !clicked->second.show_from;
        }
    }

    template<class Subscriber>
    inline void create_subscriber(const Subscriber& s) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto key = s.get_id();
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        try{
            typename Subscriber::value_type* t = nullptr;
            active[key].value_type = typeid(*t).name();
            active[key].id = key;
        } catch(...) {
            active[key].value_type = "typeid unsupported.";
        }
        if (active[key].status.empty()) {
            active[key].status = "created";
        }
        active[key].created = true;
        s.add([=](){
            if(!valid || !ofThread::isMainThread()) {return;}
            if (active.find(key) == active.end()) return;
            if(!active[key].errored && !active[key].completed) {
                active[key].status = "disposed";
            }
            silent[key] = std::move(active[key]);
            active.erase(key);
        });
    }

    template<class Observable, class Subscriber>
    inline void subscribe_enter(const Observable& o, const Subscriber& s) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto key = s.get_id();
        if(active.count(key) == 0 ) {return;}
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "subscribed";
    }
    
    template<class OperatorSource, class OperatorChain, class Subscriber, class SubscriberLifted>
    inline void lift_enter(const OperatorSource&, const OperatorChain&, const Subscriber& s, const SubscriberLifted& sl) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto fkey = sl.get_id();
        if(active.count(fkey) == 0 ) {return;}
        if ((fkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto tkey = s.get_id();
        if(active.count(tkey) == 0 ) {return;}
        if ((tkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        connect(sl, s);
        active[tkey].status = "lifted";
    }

    template<class SubscriberFrom, class SubscriberTo>
    inline void connect(const SubscriberFrom& from, const SubscriberTo& to) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto fkey = from.get_id();
        if(active.count(fkey) == 0 ) {return;}
        if ((fkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto tkey = to.get_id();
        if(active.count(tkey) == 0 ) {return;}
        if ((tkey.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[fkey].to.push_back(tkey);
        active[tkey].from.push_back(fkey);
        active[tkey].status = "operated";
    }

    template<class SubscriptionState>
    inline void unsubscribe_enter(const SubscriptionState& s) {
    }
    
    template<class Subscriber, class T>
    inline void on_next_enter(const Subscriber& s, const T&) {
        auto key = s.get_id();
        if(!valid || !ofThread::isMainThread() || active.count(key) == 0 ) {return;}
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_next", now, now});
    }
    template<class Subscriber>
    inline void on_next_return(const Subscriber& s) {
        auto key = s.get_id();
        if(!valid || !ofThread::isMainThread() || active.count(key) == 0 || active[key].marbles.empty() ) {return;}
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.back().end = now;
    }
    
    template<class Subscriber>
    inline void on_error_enter(const Subscriber& s, const std::exception_ptr&) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto key = s.get_id();
        if(active.count(key) == 0 ) {return;}
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "errored";
        active[key].errored = true;
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_error", now, now});
    }
    
    template<class Subscriber>
    inline void on_completed_enter(const Subscriber& s) {
        if(!valid || !ofThread::isMainThread()) {return;}
        auto key = s.get_id();
        if(active.count(key) == 0 ) {return;}
        if ((key.id & 0xF0000000) != 0xB0000000) std::terminate();
        active[key].status = "completed";
        active[key].completed = true;
        auto now = ofGetElapsedTimeMicros();
        active[key].marbles.push_back(marble{"on_completed", now, now});
    }
    
    void drawSubscribed(unsigned long long now, int height, ofColor glyphColor, ofColor textColor, subscribed& s, std::string prefix) {
        
        const int center = height/2;
        const int radius = height/2;
        const int lineHalfWidth = radius/4;
        const unsigned long long second = 1000000;
        const unsigned long long size = 10 * second;
        const unsigned long long end = now;
        // clamp begin to the beginning of time (0)
        const unsigned long long begin = end < size ? 0 : end - size;
        const float widthRatio = float(end - begin) / size;
        const int width = ofGetWidth();
        const int minwidth = width - std::round(width * widthRatio);

        if (!s.marbles.empty() && s.marbles.front().end < begin) {
            s.marbles.erase(
                s.marbles.begin(),
                std::lower_bound(
                    s.marbles.begin(),
                    s.marbles.end(),
                    marble{nullptr, begin, begin},
                    [](const marble& lhs, const marble& rhs){
                        return lhs.end < rhs.end;
                    }));
        }

        for (auto& m : s.marbles)
        {
            if (m.end < begin) continue;
            ofSetLineWidth(lineHalfWidth*2);
            auto xs = ofMap(m.start, end, begin, width, minwidth);
            auto xe = ofMap(m.end, end, begin, width, minwidth);
            const int middle = xs + ((xe-xs)/2);
            switch(m.method[3])
            {
                case 'n':
                    {
                        ofSetColor(glyphColor);
                        ofDrawLine(xs, center, xe, center);
                        ofDrawTriangle(middle, height, middle - radius, center, middle + radius, center);
                    }
                    break;
                case 'e':
                    {
                        ofSetColor(ofColor::red);
                        ofDrawCircle(xs + ((xe-xs)/2), height, radius * 0.5);
                        // slanted black line
                        ofSetColor(ofColor::black);
                        ofDrawLine(xs, 0, xe, height);
                    }
                    break;
                case 'c':
                    {
                        // straight black line
                        ofSetColor(ofColor::black);
                        ofDrawLine(xs, 0, xs, height);
                        ofDrawLine(xe, 0, xe, height);
                    }
                    break;
            }
        }

        s.errored ? ofSetColor(ofColor::red) : s.completed ? ofSetColor(ofColor::blue) : s.from.empty() ? ofSetColor(ofColor::limeGreen) : ofSetColor(textColor) ;
        
        std::stringstream ss;
        ss << prefix << s.id << ": of " << s.value_type << " status: " << s.status << " -> marbles: " << s.marbles.size();
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
        
        ofDrawBitmapString(ss.str(), ofPoint(radius, height, 0));
        
    }

    void draw_hud() {
        auto now = ofGetElapsedTimeMicros();
        int y = 0;
        const int height = 14;
        const int windowheight = ofGetHeight();

        std::vector<subscribed_type::value_type*> roots;
        std::transform(active.begin(), active.end(), std::back_inserter(roots),
            [&](subscribed_type::value_type& v){
                return &v;
            });

        // remove subscribers that have connections to active subscribers
        roots.erase(std::remove_if(roots.begin(), roots.end(),
            [&](subscribed_type::value_type* s){
                return std::find_if(s->second.to.begin(), s->second.to.end(),
                    [&](const rxcpp::trace_id& id){
                        return active.find(id) != active.end();
                    }) != s->second.to.end();
            }), roots.end());

        std::vector<std::deque<subscribed_type::value_type*>> streams;
        std::function<void (subscribed_type::value_type*, std::deque<subscribed_type::value_type*>&)> fill_from;
        fill_from = [&](subscribed_type::value_type* v, std::deque<subscribed_type::value_type*>& stream) {
            auto unique_from = v->second.from;
            unique_from.erase(std::unique(unique_from.begin(), unique_from.end()), unique_from.end());
            for(auto& from : unique_from) {
                auto next = active.find(from);
                if (next != active.end()) {
                    stream.push_front(&*next);
                }
            }
            for(auto& from : unique_from) {
                auto next = active.find(from);
                if (next != active.end()) {
                    fill_from(&*next, stream);
                }
            }
        };
        std::transform(roots.begin(), roots.end(), std::back_inserter(streams),
                [&](subscribed_type::value_type* v){
                    std::deque<subscribed_type::value_type*> stream;
                    stream.push_front(v);
                    fill_from(v, stream);
                    return stream;
                });

        for (auto& stream : streams)
        {
            if (stream.empty()) {
                continue;
            }

            auto& subscrib = stream.back();
            // allow the top row to be clicked to open/close
            subscrib->second.y = y;

            auto end = stream.end();
            // show ouput or full stream?
            auto cursor = subscrib->second.show_from ? stream.begin() : end - 1;

            for (;cursor != end; ++cursor)
            {
                std::string prefix;
                if (subscrib->second.show_from) {
                    if (end - 1 == cursor) {
                        prefix = "[-] ";
                    } else {
                        prefix = " v  ";
                    }
                } else if (end - 1 == cursor && stream.size() > 1) {
                    prefix = "[+] ";
                }
                auto& s = *cursor;

                if (y > windowheight) {break;}
                
                ofPushMatrix();
                ofTranslate(0, y);
                ofFill();
                
                drawSubscribed(now, height, ofColor::green, ofColor::white, s->second, prefix);
                
                ofPopMatrix();
                
                y += (height + 1);
            }
            y += (height + 1);
        }
        
        if (show_silent) {
            for (auto& s : silent)
            {
                if (y > windowheight) {break;}
                
                ofPushMatrix();
                ofTranslate(0, y);
                ofFill();
                
                s.second.y = y;
                drawSubscribed(now, height, ofColor::red, ofColor::black, s.second, "[x]");
                
                ofPopMatrix();
                
                y += (height + 1);
            }
        }
    }

    void draw() {
        if(valid) {
            if (show_stream_hud) {
                draw_hud();
            }
        }

        gui.draw();
    }
};

auto rxcpp_trace_activity(rxcpp::trace_tag) -> ofxRxTrace;

class ofxRxTraceApp : public ofBaseApp{
public:
    ofxRxTraceApp();
    
    inline void setup() {
        hud.setup();
    }
    inline void draw() {
        hud.draw();
    }
    
    ofxRxTrace& hud;
};

#endif

#include <rxcpp/rx.hpp>

#if RXCPP_VIEW_TRACE
inline ofxRxTraceApp::ofxRxTraceApp() : hud(rxcpp::trace_activity()) {}
#endif

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
