//
//  rxof_mouse.cpp
//

#include "rxof_mouse.h"

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

}