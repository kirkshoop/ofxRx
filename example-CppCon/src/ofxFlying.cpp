
#include "ofApp.h"

const int FLYING_DELAY_MS = 200;


ofxFlying::~ofxFlying()
{
}

ofxFlying::ofxFlying(ofxCommon c)
    : ofxCommon(c)
{
}

void ofxFlying::setup(float guiX) {

    message = "CppCon 2014 - The first of many!";

    gui.setup();
    
    gui.add(show_text.setup("flying text", true));
    gui.add(show_window.setup("move window", false));
    gui.add(flyingText.setup("flying text", message));

    auto sources = rx::util::to_vector({window_center(), all_movement(), just_moves(), just_drags(), never()});
    auto sourcesText = rx::util::to_vector<string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});
    
    gui.add(selected.setup("select source", 0, 0, sources.size() - 1));
    gui.add(selectedText.setup("selected source", sourcesText[0]));
    gui.add(pointsInWindow.setup("points in window", ""));

    gui.setPosition(guiX, ofGetHeight() - gui.getHeight() - 20);

    //
    // edit flying text
    //
    
    keyboard().
        releases().
        map(ofxRx::Keyboard::keyFromEvent).
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
        map(ofxRx::Keyboard::keyFromEvent).
        filter(ofxRx::Keyboard::printable).
        subscribe([this](char c){
            message.push_back(c);
            flyingText = message;
            flyingText.setSize((13 + message.size()) * 8.5, flyingText.getHeight());
        });

    auto location_points = selections.
        setup(selected).
        distinct_until_changed().
        start_with(0).
        map(
            [=](int locationSource){
                selectedText = sourcesText[selected % sourcesText.size()];
                return sources[selected % sources.size()];
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2));

    auto mouse_moves = location_points.
        combine_latest(updates().milliseconds()).
        as_dynamic();
    
    auto never_move = rxcpp::observable<>::
        never<move_record>().
        as_dynamic();

    auto move_record_source = show_text_source.
        setup(show_text).
        start_with(true).
        combine_latest(show_window_source.
            setup(show_window).
            start_with(false)).
        map(rxcpp::util::apply_to(
            [=](bool text, bool window){
                return text || window ? mouse_moves : never_move;
            })).
        switch_on_next();

    //
    // store rolling window of past points
    //
    move_record_source.
        subscribe(
            [=](const move_record& pt){
                move_window.push_back(pt);

                std::stringstream ss;
                ss << move_window.size();
                pointsInWindow = ss.str();
            });

}

void ofxFlying::draw() {
    auto now = ofGetElapsedTimeMillis();
    auto age_threshold = now - ((2 * FLYING_DELAY_MS) * message.size());
    auto keep_threshold = now - (FLYING_DELAY_MS * message.size());
    
    if (!move_window.empty() && age_threshold < now && std::get<1>(move_window.front()) < age_threshold) {
        move_window.erase(
            move_window.begin(),
            std::lower_bound(
                move_window.begin(), move_window.end(),
                move_record(ofPoint(), keep_threshold),
                [&](const move_record& lhs, const move_record& rhs){
                    return std::get<1>(lhs) < std::get<1>(rhs);
                }));
    }

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
        auto pt_end = move_window.end();
        auto index = 0;
        for (auto& c : message) {
            auto pt_cursor = move_window.begin();
            
            //
            // find point to place this character
            //
            ofPoint at;
            // choose a time in the past
            auto time = now - (FLYING_DELAY_MS * index);
            // search through the past for the point value at the selected time
            pt_cursor = std::upper_bound(
                pt_cursor, pt_end,
                move_record(ofPoint(), time),
                [&](const move_record& lhs, const move_record& rhs){
                    return std::get<1>(lhs) < std::get<1>(rhs);
                });
            if (pt_cursor == pt_end) {
                // extract the first point
                at = std::get<0>(move_window.back());
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
