#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxRx.h"
#include "ofxRxHttp.h"
namespace rx=ofxRx::rx;

class ofApp : public ofBaseApp{
	public:
        ofApp();
		void setup();
		void draw();
		
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    ofx::rx::HttpClient http;
    std::ostringstream progress;
    rx::subjects::subject<std::string> sub_get;
    rx::subscriber<std::string> dest_get;
    struct HttpGet
    {
        float progress;
        std::string url;
        std::string message;
    };
    std::list<HttpGet> gets;
};
