#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "rxof_mouse.h"

template<class T>
struct ofx_observe
{
    ofx_observe()
        :
        dest_t(rx::make_observer_dynamic<T>(sub_t.get_subscriber().get_observer()))
    {
    }
    
    template<class OfxControl>
    rx::observable<T> setup(OfxControl& c) {
        c.addListener(this, &ofx_observe::notification);
        return sub_t.get_observable().as_dynamic();
    }
    
    void notification(T & t){
        dest_t.on_next(t);
    }
    
private:
    rx::subjects::subject<T> sub_t;
    rx::observer<T> dest_t;
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    rxof::Mouse mouse;

	ofxToggle show_circle;
    ofxToggle orbit_circle;
	ofxToggle show_text;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    
    ofx_observe<int> selections;

	ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    rx::subjects::subject<long> updates;
    rx::util::detail::maybe<rx::observer<long>> dest_updates;
    rx::subjects::subject<int> key_releases;
    rx::util::detail::maybe<rx::observer<int>> dest_key_releases;
    ofPoint center;
    std::string message;
    std::deque<std::tuple<ofPoint, long>> move_window;
};
