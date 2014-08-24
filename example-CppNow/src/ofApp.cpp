#include "ofApp.h"

ofApp::~ofApp()
{
}

ofApp::ofApp()
    : circle(common)
    , flying(common)
{
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetCircleResolution(100);
    ofEnableSmoothing();
    
    common.setup();
    
    circle.setup();
    flying.setup();
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::gray, ofColor::gray);

    ofFill();

    common.draw();

    circle.draw();
    flying.draw();
}

