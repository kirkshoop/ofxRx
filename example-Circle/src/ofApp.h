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
    rxof::Updates updates;

	ofxToggle show_circle;
    ofxToggle orbit_circle;
    ofxFloatSlider circle_radius;
    ofxFloatSlider orbit_radius;
    ofxFloatSlider orbit_period;
    ofxIntSlider selected;
    ofxLabel selectedText;
    
    rxof::observe_source<int> selections;

	ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    ofPoint center;
};
