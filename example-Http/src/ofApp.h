#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxRx.h"
#include "ofxRxHttp.h"
namespace rx=ofxRx::rx;

#include "HttpGet.h"

class ofApp : public ofBaseApp{
	public:
        ofApp();
		void setup();
		void draw();
		
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    static const int itemHeight = 80;
    
    ofx::rx::HttpClient http;
    rx::subjects::subject<std::string> sub_get;
    rx::subscriber<std::string> dest_get;
    std::list<HttpGet> gets;
};
