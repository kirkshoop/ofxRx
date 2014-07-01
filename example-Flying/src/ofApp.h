#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxRx.h"
namespace rx=ofxRx::rx;

class ofApp : public ofBaseApp{
	public:
        ~ofApp();
        ofApp();

        void setup();
		void draw();
		
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    ofxRx::Mouse mouse;
    ofxRx::Keyboard keyboard;
    ofxRx::Updates updates;

	ofxToggle show_text;
	ofxToggle show_window;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    ofxLabel pointsInWindow;
    
    ofxRx::observe_source<int> selections;

	ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    rx::subscriber<rx::observable<ofPoint>> dest_center;
    std::string message;
    typedef std::tuple<ofPoint, unsigned long long> move_record;
    std::deque<move_record> move_window;
};
