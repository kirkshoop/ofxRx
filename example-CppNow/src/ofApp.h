#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxRx.h"
namespace rx=ofxRx::rx;

#include "ofxCommon.h"

#include "ofxCircle.h"
#include "ofxFLying.h"

class ofApp : public ofBaseApp{
	public:
        ~ofApp();
        ofApp();

		void setup();
		void draw();
		
    ofxCommon common;
    ofxCircle circle;
    ofxFlying flying;
};


