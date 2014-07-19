#include "ofApp.h"


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
                // add state object to track this url
                gets.push_back(HttpGet(std::move(url), itemHeight));
                auto last = --gets.end();
                return last->get(http).
                    finally([=](){
                        // image is loaded, fade image and then remove it.
                        rx::observable<>::interval(ofx::rx::make_update().now() + milliseconds(250),
                            milliseconds(100),
                            ofx::rx::serialize_update()).
                            take(10).
                            subscribe(
                                [=](long){
                                    last->imageOpacity = std::max(0, last->imageOpacity - (255 / 8));
                                },
                                [=](){
                                    gets.erase(last);
                                });
                    });
            },
            rx::util::pack()
        ).
        subscribe();

    // 10 times a second; submit an url from the ofupdate event, stop after 10 urls have been submitted
    rx::observable<>::interval(ofx::rx::synchronize_update().now(), milliseconds(200), ofx::rx::synchronize_update()).
        map([this](long){
            return std::string("http://peach.blender.org/wp-content/uploads/poster_rodents_small.jpg");
        }).
        take(20).
        subscribe(dest_get);
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::white, ofColor::gray);

    ofFill();

    int y = 0;
    int height = itemHeight;
    int width = ofGetWidth();
    
    for (auto& get : gets)
    {
        if (y > ofGetHeight()) {break;}

        ofPushMatrix();
        ofTranslate(0, y);
        ofFill();

        get.draw(width, height);

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