#include "ofApp.h"

#include "Step1.h"
#include "Step2.h"
#include "Step3.h"
#include "Step4.h"

const int FLYING_DELAY_MS = 200;

ofApp::~ofApp()
{
}

ofApp::ofApp()
#if RXCPP_VIEW_TRACE
    :
    vt(rx::trace_activity())
#endif
{
}

rxcpp::observable<float>
ofApp::timeInPeriodFromMilliseconds(
    rxcpp::observable<unsigned long long> timeInMilliseconds){
    return timeInMilliseconds.
        map(
            [this](unsigned long long tick){
                // map the tick into the range 0.0-1.0
                return ofMap(tick % int(orbit_period * 1000), 0, int(orbit_period * 1000), 0.0, 1.0);
            });
}

rxcpp::observable<ofPoint>
ofApp::orbitPointsFromTimeInPeriod(
    rxcpp::observable<float> timeInPeriod){
    return timeInPeriod.
        map(
            [this](float t){
                // map the time value to a point on a circle
                return ofPoint(orbit_radius * std::cos(t * 2 * 3.14), orbit_radius * std::sin(t * 2 * 3.14));
            });
}

//static
ofPoint
ofApp::pointFromMouse(
    ofMouseEventArgs e){
    return ofPoint(e.x, e.y);
};


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetCircleResolution(100);
    ofEnableSmoothing();
    
    mouse.setup();
    updates.setup();
    gui.setup();

    gui.add(show_circle.setup("circle", true));
    gui.add(orbit_circle.setup("circle orbits", true));
    gui.add(circle_radius.setup("circle radius", 20.0, 10.0, 600.0));
    gui.add(orbit_radius.setup("orbit radius", 50.0, 10.0, 600.0));
    gui.add(orbit_period.setup("orbit period", 1.0, 0.5, 5.0));

#if RXCPP_VIEW_TRACE
    vt.setup();
#endif

#define OFX_CIRCLE_STEP 1
    
#if OFX_CIRCLE_STEP == 1
    step_one_setup(this);
#elif OFX_CIRCLE_STEP == 2
    step_two_setup(this);
#elif OFX_CIRCLE_STEP == 3
    step_three_setup(this);
#elif OFX_CIRCLE_STEP == 4
    step_four_setup(this);
#endif
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::gray, ofColor::gray);

    ofFill();

#if RXCPP_VIEW_TRACE
    vt.draw();
#endif
    
    //
    // display circle
    //

	ofSetColor(ofColor(0x66,0x33,0x99));

    if (show_circle) {
        ofCircle(center, circle_radius);
    }

    gui.draw();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    if (selected == 0) {
        selected = 0;
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}