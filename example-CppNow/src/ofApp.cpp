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
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::gray, ofColor::gray);

    ofFill();

    common.draw();

    images.draw();
    circle.draw();
    flying.draw();
}

