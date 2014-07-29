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
    ofxRx::Updates updates;

	ofxToggle show_circle;
    ofxToggle orbit_circle;
    ofxFloatSlider circle_radius;
    ofxFloatSlider orbit_radius;
    ofxFloatSlider orbit_period;
    ofxIntSlider selected;
    ofxLabel selectedText;
    
    ofxRx::observe_source<int> selections;
    ofxRx::observe_source<bool> orbits;

	ofxPanel gui;
    
#if RXCPP_VIEW_TRACE
    ofxRxTrace& vt;
#endif 

    ofPoint center;
};
