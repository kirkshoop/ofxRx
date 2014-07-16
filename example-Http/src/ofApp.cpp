#include "ofApp.h"

const int FLYING_DELAY_MS = 200;

ofApp::ofApp():
    dest_get(sub_get.get_subscriber().as_dynamic())
{
}

//--------------------------------------------------------------
void ofApp::setup(){

    using namespace std::chrono;

    ofEnableSmoothing();
    
    // for every url submitted, download and track progress
    sub_get.get_observable().
        observe_on(ofx::rx::synchronize_update()).
        flat_map(
            [this](std::string url){

                gets.push_back(HttpGet());
                auto* state = std::addressof(gets.back());
                state->url = url;
                state->progress = 0.0;
                state->message = "pending...";

                return http.get(url).
                    flat_map(
                        [](ofxRx::HttpProgress hp){
                            return hp.response().
                                map([](ofx::HTTP::ClientResponseProgressArgs rp){
                                    return rp.getProgress();
                                }).
                                as_dynamic();
                        },
                        [](ofxRx::HttpProgress hp, float p){
                            return p;
                        }).
                    subscribe_on(rx::serialize_event_loop()).
                    observe_on(ofx::rx::synchronize_update()).
                    lift(
                        [=](rx::subscriber<float> out){
                            return rx::make_subscriber<float>(
                                [=](float p){
                                    state->progress = p;
                                    out.on_next(p);
                                },
                                [=](std::exception_ptr ex){
                                    state->message = "failed.";
                                    out.on_completed();
                                },
                                [=](){
                                    state->message = "completed";
                                    out.on_completed();
                                });
                        }).
                    as_dynamic();
            },
            rx::util::pack()).
        subscribe();

    // once a second submit an url, stop after 10 urls have been submitted
    rx::observable<>::interval(ofx::rx::synchronize_update().now(), seconds(1), ofx::rx::synchronize_update()).
        map([this](long){
            return std::string("http://peach.blender.org/wp-content/uploads/big_big_buck_bunny.jpg");
        }).
        take(10).
        subscribe(dest_get);
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::white, ofColor::gray);

    ofFill();

    int y = 0;
    int height = 20;
    int width = ofGetWidth();
    
    for (auto& get : gets)
    {
        if (y > ofGetHeight()) {break;}

        ofPushMatrix();
        ofTranslate(0, y);
        ofFill();

        ofSetColor(0, 255, 0, 50);
        if (get.message == "failed.") {
            ofSetColor(255, 0, 0);
        } else if (get.message == "completed.") {
            ofSetColor(255, 255, 0, 50);
        }
        
        ofRect(0, 0, width, height);
        
        if (get.progress > 0)
        {
            ofFill();
            ofSetColor(255, 255, 0, 75);
            ofRect(0, 0, get.progress * width, height);
        }
        
        ofSetColor(255);
        
        std::stringstream ss;
        
        ss << get.url + " " << (get.progress * 100) << "%: " << get.message;
        
        ofDrawBitmapString(ss.str(), ofPoint(10, 14, 0));
        
        ofPopMatrix();
        
        y += (height + 1);
    }
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