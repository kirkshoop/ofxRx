
#include "ofApp.h"

const int FLYING_DELAY_MS = 200;


ofxFlying::~ofxFlying()
{
    dest_center.on_completed();

}

ofxFlying::ofxFlying(ofxCommon c)
    : ofxCommon(c)
    , dest_center(center_source.get_subscriber().as_dynamic())
{
}

void ofxFlying::setup() {

    message = "Time flies like an arrow";

    gui.setup();
    
    gui.add(show_text.setup("flying text", true));
    gui.add(show_window.setup("move window", false));
    gui.add(flyingText.setup("flying text", message));

    auto sources = rx::util::to_vector({window_center(), all_movement(), just_moves(), just_drags(), never()});
    auto sourcesText = rx::util::to_vector<string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});
    
    gui.add(selected.setup("select source", 0, 0, sources.size() - 1));
    gui.add(selectedText.setup("selected source", sourcesText[0]));
    gui.add(pointsInWindow.setup("points in window", ""));

    //
    // edit flying text
    //
    
    keyboard().
        releases().
        map([](ofKeyEventArgs a){return a.key;}).
        filter([](int key){ return key == OF_KEY_BACKSPACE; }).
        subscribe([this](int){
            if (!message.empty()) {
                message.erase(--message.end());
            }
            flyingText = message;
            flyingText.setSize((13 + message.size()) * 8.5, flyingText.getHeight());
        });
    
    keyboard().
        releases().
        map([](ofKeyEventArgs a){return a.key;}).
        filter([](int key){ return !(key & OF_KEY_MODIFIER) && key != OF_KEY_BACKSPACE; }).
        subscribe([this](char c){
            message.push_back(c);
            flyingText = message;
            flyingText.setSize((13 + message.size()) * 8.5, flyingText.getHeight());
        });

    auto location_points = selections.setup(selected).
        distinct_until_changed().
        start_with(0).
        map(
            [=](int locationSource){
                selectedText = sourcesText[selected % sourcesText.size()];
                return sources[selected % sources.size()];
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2));

    //
    // collect rolling window of past points
    //

    location_points.
        combine_latest(updates().milliseconds()).
        subscribe(
            [=](const move_record& pt){
                move_window.push_back(pt);
                while(move_window.size() > 1 &&
                    std::get<1>(pt) > (message.size() * FLYING_DELAY_MS) &&
                    std::get<1>(move_window.front()) < std::get<1>(pt) - (message.size() * FLYING_DELAY_MS)) {
                    move_window.pop_front();
                }
                std:stringstream ss;
                ss << move_window.size();
                pointsInWindow = ss.str();
            });

}

void ofxFlying::draw() {
    //
    // display move window
    //
    
    if (show_window) {
        ofSetColor(ofColor(0x66,0x33,0x99));
        
        for(auto& m : move_window) {
            ofCircle(std::get<0>(m), 4.0);
        }
    }
    
    //
    // display flying text
    //
    
    if (show_text && !move_window.empty()) {
        auto pt_cursor = move_window.rbegin();
        auto pt_end = move_window.rend();
        auto index = 0;
        auto now = ofGetElapsedTimeMillis();
        for (auto& c : message) {
            
            //
            // find point to place this character
            //
            ofPoint at;
            // choose a time in the past
            auto time = now - (FLYING_DELAY_MS * index);
            // search through the past for the point value at the selected time
            pt_cursor = std::find_if(pt_cursor, pt_end,
                                     [&](const move_record& tp){
                                         return time > std::get<1>(tp);
                                     });
            if (pt_cursor == pt_end) {
                // too far in the past, bail
                break;
            } else {
                // extract the point
                at = std::get<0>(*pt_cursor);
            }
            
            //
            //draw the character at the point + the width of the preceeding characters
            //
            
            std::string s;
            s.push_back(c);
            ofDrawBitmapStringHighlight(s, at.x + (index * 15), at.y);
            
            ++index;
        }
    }
    
    gui.draw();
}
