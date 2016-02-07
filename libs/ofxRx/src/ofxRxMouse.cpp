
#include <ofxRx.h>

namespace ofx {

namespace rx {

Mouse::Mouse()
    :
    dest_moves(sub_moves.get_subscriber().as_dynamic()),
    dest_drags(sub_drags.get_subscriber().as_dynamic()),
    dest_presses(sub_presses.get_subscriber().as_dynamic()),
    dest_releases(sub_releases.get_subscriber().as_dynamic()),
    dest_scrolls(sub_scrolls.get_subscriber().as_dynamic()),
    dest_enters(sub_enters.get_subscriber().as_dynamic()),
    dest_exits(sub_exits.get_subscriber().as_dynamic())
{
    registered = false;
}

Mouse::~Mouse() {
    clear();
    dest_moves.on_completed();
    dest_drags.on_completed();
    dest_presses.on_completed();
    dest_releases.on_completed();
    dest_scrolls.on_completed();
    dest_enters.on_completed();
    dest_exits.on_completed();
}

//static
ofPoint Mouse::pointFromEvent(ofMouseEventArgs e){
    return ofPoint(e.x, e.y);
};

void Mouse::setup(){
    if(!registered) {
        ofAddListener(ofEvents().mouseMoved,this,&Mouse::mouseMoved);
        ofAddListener(ofEvents().mouseDragged,this,&Mouse::mouseDragged);
        ofAddListener(ofEvents().mousePressed,this,&Mouse::mousePressed);
        ofAddListener(ofEvents().mouseReleased,this,&Mouse::mouseReleased);
        ofAddListener(ofEvents().mouseScrolled,this,&Mouse::mouseScrolled);
        ofAddListener(ofEvents().mouseEntered,this,&Mouse::mouseEntered);
        ofAddListener(ofEvents().mouseExited,this,&Mouse::mouseExited);
        registered = true;
    }
}

void Mouse::clear() {
    if(registered) {
        ofRemoveListener(ofEvents().mouseMoved, this, &Mouse::mouseMoved);
        ofRemoveListener(ofEvents().mouseDragged, this, &Mouse::mouseDragged);
        ofRemoveListener(ofEvents().mousePressed, this, &Mouse::mousePressed);
        ofRemoveListener(ofEvents().mouseReleased, this, &Mouse::mouseReleased);
        ofRemoveListener(ofEvents().mouseScrolled, this, &Mouse::mouseScrolled);
        ofRemoveListener(ofEvents().mouseEntered, this, &Mouse::mouseEntered);
        ofRemoveListener(ofEvents().mouseExited, this, &Mouse::mouseExited);
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
rx::observable<ofMouseEventArgs> Mouse::scrolls(){
    return sub_scrolls.get_observable();
}
rx::observable<ofMouseEventArgs> Mouse::enters(){
    return sub_enters.get_observable();
}
rx::observable<ofMouseEventArgs> Mouse::exits(){
    return sub_exits.get_observable();
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
void Mouse::mouseScrolled(ofMouseEventArgs & args){
    dest_scrolls.on_next(args);
}
void Mouse::mouseEntered(ofMouseEventArgs & args){
    dest_enters.on_next(args);
}
void Mouse::mouseExited(ofMouseEventArgs & args){
    dest_exits.on_next(args);
}

}

}