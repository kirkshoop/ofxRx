#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    selected = 0;
    
    dest_updates.reset(rx::make_observer_dynamic<long>(updates.get_subscriber().get_observer()));

    mouse.setup();

    auto orbit_points = updates.get_observable().
        map([](long tick){return ofMap(tick % 1000, 0, 1000, 0.0, 1.0);}).
        map([](float t){return ofPoint(50 * std::cos(t * 2 * 3.14), 50 * std::sin(t * 2 * 3.14));}).
        as_dynamic();

    auto center_points = center_source.get_observable().
        switch_on_next().
        as_dynamic();
    
    orbit_points.
        combine_latest(center_points).
        map(rx::util::apply_to([](ofPoint o, ofPoint c){return o + c;})).
        subscribe(
              [this](ofPoint c){
                  center = c;
              });
    
    auto pointFromMouse = [](ofMouseEventArgs e){
        return ofPoint(e.x, e.y);
    };

    auto window_center = rx::observable<>::just(ofPoint((ofGetWidth()/2) - 20, (ofGetHeight()/2) - 20)).
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

    auto sources = rx::util::to_vector({window_center, all_movement, just_moves, just_drags, never});

    auto dest_center = center_source.get_subscriber().get_observer();

    dest_center.on_next(window_center);
    
    mouse.releases().
        subscribe(
            [=](ofMouseEventArgs e){
                selected += e.x < (ofGetWidth()/4) ? -1 : e.x < (ofGetWidth() - (ofGetWidth()/4)) ? 0 : 1;
                dest_center.on_next(sources[selected % sources.size()]);
            });
}

//--------------------------------------------------------------
void ofApp::update(){
    dest_updates->on_next(ofGetElapsedTimeMillis());
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofCircle(center, 20);
    
    auto sources = rx::util::to_vector<string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});

    string msg = "releasing a click or drag on the left side of the window causes the selected source to change to the previous\n";
    msg += "releasing a click or drag on the right side of the window causes the selected source to change to the next\n";
    msg += "releasing the mouse in the middle will not change the selected source\n";
    msg += "the selected source is: " + sources[selected % sources.size()];
    ofDrawBitmapStringHighlight(msg, 30,30);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}