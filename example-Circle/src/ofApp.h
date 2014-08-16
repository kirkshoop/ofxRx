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
    
    rxcpp::observable<float> timeInPeriodFromMilliseconds(rxcpp::observable<unsigned long long> timeInMilliseconds);
    rxcpp::observable<ofPoint> orbitPointsFromTimeInPeriod(rxcpp::observable<float> timeInPeriod);

    static ofPoint pointFromMouse(ofMouseEventArgs e);


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
    ofxRx::observe_source<bool> locations;

	ofxPanel gui;
    
#if RXCPP_VIEW_TRACE
    ofxRxTrace& vt;
#endif 

    ofPoint center;
};


