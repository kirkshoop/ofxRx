
#include "ofApp.h"

ofxCommon::type::~type()
{
}

ofxCommon::type::type()
#if RXCPP_VIEW_TRACE
    : vt(rx::trace_activity())
#endif
{
}

ofxCommon::~ofxCommon()
{
}

ofxCommon::ofxCommon()
    : shared(std::make_shared<type>())
{
}

rx::observable<ofPoint> ofxCommon::window_center() {
    return rx::observable<>::
        defer(
              [](){
                  return rx::observable<>::
                  just(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
              });
}

rx::observable<ofPoint> ofxCommon::all_movement() {
    return rx::observable<>::
        from(mouse().moves(), mouse().drags()).
        merge().
        map(ofxRx::Mouse::pointFromEvent).
        as_dynamic();
}

rx::observable<ofPoint> ofxCommon::just_moves() {
    return mouse().
        moves().
        map(ofxRx::Mouse::pointFromEvent);
}

rx::observable<ofPoint> ofxCommon::just_drags() {
    return mouse().
        drags().
        map(ofxRx::Mouse::pointFromEvent);
}

rx::observable<ofPoint> ofxCommon::never() {
    return rx::observable<>::
    never<ofPoint>();
}

void ofxCommon::setup() {
    shared->mouse.setup();
    shared->keyboard.setup();
    shared->updates.setup();

#if RXCPP_VIEW_TRACE
    shared->vt.setup();
#endif
}

void ofxCommon::draw() {
#if RXCPP_VIEW_TRACE
    shared->vt.draw();
#endif
}

