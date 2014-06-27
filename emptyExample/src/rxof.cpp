//
//  rxof_mouse.cpp
//

#include "rxof.h"

namespace rxof {

Mouse::Mouse()
    :
    dest_moves(rx::make_observer_dynamic<ofMouseEventArgs>(sub_moves.get_subscriber().get_observer())),
    dest_drags(rx::make_observer_dynamic<ofMouseEventArgs>(sub_drags.get_subscriber().get_observer())),
    dest_presses(rx::make_observer_dynamic<ofMouseEventArgs>(sub_presses.get_subscriber().get_observer())),
    dest_releases(rx::make_observer_dynamic<ofMouseEventArgs>(sub_releases.get_subscriber().get_observer()))
{
    registered = false;
}

Mouse::~Mouse() {
    clear();
    dest_moves.on_completed();
    dest_drags.on_completed();
    dest_presses.on_completed();
    dest_releases.on_completed();
}

void Mouse::setup(){
    if(!registered) {
        ofRegisterMouseEvents(this);
        registered = true;
    }
}

void Mouse::clear() {
    if(registered) {
        ofUnregisterMouseEvents(this);
        registered = false;
    }
}

rx::observable<ofMouseEventArgs> Mouse::moves(){
    return sub_moves.get_observable();
}
rx::observable<ofMouseEventArgs> Mouse::drags(){
    return sub_drags.get_observable();
}
rx::observable<ofMouseEventArgs> Mouse::presses(){
    return sub_presses.get_observable();
}
rx::observable<ofMouseEventArgs> Mouse::releases(){
    return sub_releases.get_observable();
}

void Mouse::mouseMoved(ofMouseEventArgs & args){
    dest_moves.on_next(args);
}
void Mouse::mouseDragged(ofMouseEventArgs & args){
    dest_drags.on_next(args);
}
void Mouse::mousePressed(ofMouseEventArgs & args){
    dest_presses.on_next(args);
}
void Mouse::mouseReleased(ofMouseEventArgs & args){
    dest_releases.on_next(args);
}

    
    
Keyboard::Keyboard()
:
dest_presses(rx::make_observer_dynamic<ofKeyEventArgs>(sub_presses.get_subscriber().get_observer())),
dest_releases(rx::make_observer_dynamic<ofKeyEventArgs>(sub_releases.get_subscriber().get_observer()))
{
    registered = false;
}

Keyboard::~Keyboard() {
    clear();
    dest_presses.on_completed();
    dest_releases.on_completed();
}

void Keyboard::setup(){
    if(!registered) {
        ofRegisterKeyEvents(this);
        registered = true;
    }
}

void Keyboard::clear() {
    if(registered) {
        ofUnregisterKeyEvents(this);
        registered = false;
    }
}

rx::observable<ofKeyEventArgs> Keyboard::presses(){
    return sub_presses.get_observable();
}
rx::observable<ofKeyEventArgs> Keyboard::releases(){
    return sub_releases.get_observable();
}

void Keyboard::keyPressed(ofKeyEventArgs& a){
    dest_presses.on_next(a);
}
void Keyboard::keyReleased(ofKeyEventArgs& a){
    dest_releases.on_next(a);
}

    
    
Updates::~Updates() {
    clear();
}
Updates::Updates()
:
dest_updates(rx::make_observer_dynamic<ofEventArgs>(sub_updates.get_subscriber().get_observer()))
{
    registered = false;
}

void Updates::setup() {
    ofAddListener(ofEvents().update, this, &Updates::update);
    registered = true;
}
void Updates::clear() {
    ofRemoveListener(ofEvents().update, this, &Updates::update);
    registered = false;
}

rx::observable<ofEventArgs> Updates::events()
{
    return sub_updates.get_observable().as_dynamic();
}

rx::observable<unsigned long long> Updates::milliseconds()
{
    return sub_updates.
    get_observable().
    map([](const ofEventArgs&){return ofGetElapsedTimeMillis();}).
    as_dynamic();
}

rx::observable<unsigned long long> Updates::microseconds()
{
    return sub_updates.
    get_observable().
    map([](const ofEventArgs&){return ofGetElapsedTimeMicros();}).
    as_dynamic();
}

rx::observable<float> Updates::floats()
{
    return sub_updates.
    get_observable().
    map([](const ofEventArgs&){return ofGetElapsedTimef();}).
    as_dynamic();
}

void Updates::update(ofEventArgs& a){
    dest_updates.on_next(a);
}

}