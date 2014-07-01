#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxRx.h"
namespace rx=ofxRx::rx;

class ofApp : public ofBaseApp{
	public:
		void setup();
		void draw();
		
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    ofxRx::Updates updates;
    std::string updates_count;
    std::string thread_count;
};
