
#include <ofxRx.h>

namespace ofx {

namespace rx {

    
Keyboard::Keyboard()
:
dest_presses(sub_presses.get_subscriber().as_dynamic()),
dest_releases(sub_releases.get_subscriber().as_dynamic())
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


}

}