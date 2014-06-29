#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxRx.h"

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

	ofxToggle show_text;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    
    rxof::observe_source<int> selections;

	ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    std::string message;
    typedef std::tuple<ofPoint, unsigned long long> move_record;
    std::deque<move_record> move_window;
};
