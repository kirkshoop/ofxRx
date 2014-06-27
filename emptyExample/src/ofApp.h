#pragma once

#include "rxof_mouse.h"

#include "ofMain.h"

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
    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    rx::subjects::subject<long> updates;
    rx::util::detail::maybe<rx::observer<long>> dest_updates;
    ofPoint center;
    int selected;
    std::string message;
    std::deque<std::tuple<ofPoint, long>> move_window;
};
