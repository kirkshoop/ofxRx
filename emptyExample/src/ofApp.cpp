#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    selected = 0;
    
    message = "Time flies like an arrow";
    
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
    combine_latest(std::plus<ofPoint>(), center_points).
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

    all_movement.
        combine_latest(updates.get_observable()).
        subscribe([=](const std::tuple<ofPoint, long >& pt){
            move_window.push_back(pt);
            while(move_window.size() > 1 && std::get<1>(move_window.front()) < std::get<1>(pt) - 10000) {
                move_window.pop_front();
            }
        });

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
    
    if (!move_window.empty()) {
        auto pt_cursor = move_window.begin();
        auto index = 0;
        auto now = ofGetElapsedTimeMillis();
        for (auto& c : message) {
            std::string s;
            s.push_back(c);
            ofPoint at;
            long tick;
            std::tie(at, tick) = move_window.front();
            auto time = now - (200 * index);
            auto found = std::find_if(move_window.rbegin(), move_window.rend(),
                [&](std::tuple<ofPoint, long> tp){
                    return time > std::get<1>(tp);
                });
            if (found == move_window.rend()) {
                std::tie(at, tick) = move_window.back();
            } else {
                std::tie(at, tick) = *found;
            }
            ofDrawBitmapStringHighlight(s, at.x + (index * 15), at.y);
            ++index;
        }
    }
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