#include "ofApp.h"

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

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetCircleResolution(100);
    ofEnableSmoothing();
    
    mouse.setup();
    updates.setup();
    gui.setup();

    //
    // calculate orbit position based on time
    //
    auto orbit_points = updates.milliseconds().
        map(
            [this](unsigned long long tick){
                // map the tick into the range 0.0-1.0
                return ofMap(tick % int(orbit_period * 1000), 0, int(orbit_period * 1000), 0.0, 1.0);
            }).
        map(
            [this](float t){
                // map the time value to a point on a circle
                return ofPoint(orbit_radius * std::cos(t * 2 * 3.14), orbit_radius * std::sin(t * 2 * 3.14));
            });

    //
    // define point sources
    //

    auto pointFromMouse = [](ofMouseEventArgs e){
        return ofPoint(e.x, e.y);
    };
    
    auto window_center = rx::observable<>::defer(
        [](){
            return rx::observable<>::just(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
        }).
        as_dynamic();
    
    auto all_movement = rx::observable<>::from(mouse.moves(), mouse.drags()).
        merge().
        map(pointFromMouse).
        as_dynamic();

    auto just_moves = mouse.moves().
        map(pointFromMouse).
        as_dynamic();

    auto just_drags = mouse.drags().
        map(pointFromMouse).
        as_dynamic();
    
    auto never = rx::observable<>::never<ofPoint>().
        as_dynamic();

    //
    // display gui
    //
    
    auto sources = rx::util::to_vector({window_center, all_movement, just_moves, just_drags, never});
    auto sourcesText = rx::util::to_vector<string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});

    gui.add(show_circle.setup("circle", true));
    gui.add(orbit_circle.setup("circle orbits", true));
    gui.add(circle_radius.setup("circle radius", 20.0, 10.0, 600.0));
	gui.add(orbit_radius.setup("orbit radius", 50.0, 10.0, 600.0));
    gui.add(orbit_period.setup("orbit period", 1.0, 0.5, 5.0));
	gui.add(selected.setup("select source", 0, 0, sourcesText.size() - 1));
    gui.add(selectedText.setup("selected source", ""));

    //
    // use most recently selected source of points
    //
    
    selections.setup(selected).
        publish(0).ref_count(). // selected is zero at start, also remember the last value
        // combines events from the ui controls to select a stream of points
        combine_latest(
            [=](int selected, bool orbits){

                // update ui text
                string msg = sourcesText[selected % sourcesText.size()];
                selectedText = msg;
                selectedText.setSize((17 + msg.size()) * 8.5, selectedText.getHeight());

                auto selected_source = sources[selected % sources.size()].
                    // use the mouse position at start, also remember the last value
                    publish(ofPoint(ofGetMouseX(), ofGetMouseY())).ref_count();

                if (orbits) {
                    // add the selected source and the orbit source together
                    return orbit_points.
                        combine_latest(
                            std::plus<ofPoint>(),
                            selected_source).
                        as_dynamic();
                }

                // use only the selected source
                return selected_source.
                    as_dynamic();
            }, orbits.setup(orbit_circle).
                publish(true).ref_count() // orbits is true at start, also remember the last value
            ).
        // let each new source selection replace the last source
        switch_on_next().
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                center = c;
            });
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
        auto window_center = rx::observable<>::just(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
        orbit_circle = (bool)orbit_circle;
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}