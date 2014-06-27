#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "rxof.h"

template<class T>
struct ofx_observe
{
    ofx_observe()
        :
        dest_t(rx::make_observer_dynamic<T>(sub_t.get_subscriber().get_observer()))
    {
        registered = false;
    }
    
    template<class OfxControl>
    rx::observable<T> setup(OfxControl& c) {
        if (!registered) {
            c.addListener(this, &ofx_observe::notification);
        }
        return sub_t.get_observable().as_dynamic();
    }
    
    void notification(T & t){
        dest_t.on_next(t);
    }
    
private:
    bool registered;
    rx::subjects::subject<T> sub_t;
    rx::observer<T> dest_t;
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void draw();
		
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    rxof::Mouse mouse;
    rxof::Keyboard keyboard;
    rxof::Updates updates;

	ofxToggle show_circle;
    ofxToggle orbit_circle;
	ofxToggle show_text;
    ofxFloatSlider circle_radius;
    ofxFloatSlider orbit_radius;
    ofxFloatSlider orbit_period;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    
    ofx_observe<int> selections;

	ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    ofPoint center;
    std::string message;
    typedef std::tuple<ofPoint, unsigned long long> move_record;
    std::deque<move_record> move_window;
};
