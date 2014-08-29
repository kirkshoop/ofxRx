#include "ofApp.h"

ofApp::~ofApp()
{
}

ofApp::ofApp()
    : circle(common)
    , flying(common)
    , images(common)
{
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetCircleResolution(100);
    ofEnableSmoothing();

    common.setup();
    
    circle.setup(50);
    images.setup(300);
    flying.setup(550);

    gui.setup();
    
    gui.add(show_circle.setup("show circle", false));
    gui.add(show_text.setup("show text", false));
    gui.add(show_images.setup("show images", false));
    gui.add(show_debug.setup("show debug", false));
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::gray, ofColor::gray);

    ofFill();

    if(show_debug) {
        common.draw();
    }

    if(show_images) {
        images.draw();
    }
    if (show_circle) {
        circle.draw();
    }
    if (show_text) {
        flying.draw();
    }

    gui.draw();
}

