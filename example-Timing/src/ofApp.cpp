#include "ofApp.h"

const int FLYING_DELAY_MS = 200;

//--------------------------------------------------------------
void ofApp::setup(){

    using namespace std::chrono;

    ofEnableSmoothing();

    auto start = ofx::rx::make_update().now();
    auto step = milliseconds(500);
    
    auto stringFromCount = [=](long count){
        std::stringstream ss;
        auto tick = ofx::rx::make_update().now();
        auto second_ms = duration_cast<milliseconds>(seconds(1));
        auto actual_ms = duration_cast<milliseconds>(tick - start);
        auto expected_ms = duration_cast<milliseconds>(count * step);
        ss << std::fixed
            << count
            << " timing -> expected: " << float(expected_ms.count())/second_ms.count()
            << "s, actual: " << float(actual_ms.count())/second_ms.count()
            << "s, drift: " << float(actual_ms.count() - expected_ms.count())/second_ms.count()
            << "s";
        return ss.str();
    };

    rx::observable<>::interval(start + step, step, ofx::rx::serialize_update()).
        subscribe(
            [=](long count){
                updates_count = stringFromCount(count);
            });

    rx::observable<>::interval(start + step, step).
        subscribe_on(rx::serialize_new_thread()).
        observe_on(ofx::rx::serialize_update()).
        subscribe(
            [=](long count){
                thread_count = stringFromCount(count);
            });
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::white, ofColor::gray);

    ofFill();

    ofDrawBitmapStringHighlight(updates_count, 100, 20);
    ofDrawBitmapStringHighlight(thread_count, 100, 80);
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