
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
        ofAddListener(ofEvents().keyPressed,this,&Keyboard::keyPressed);
        ofAddListener(ofEvents().keyReleased,this,&Keyboard::keyReleased);
        registered = true;
    }
}

void Keyboard::clear() {
    if(registered) {
        ofRemoveListener(ofEvents().keyPressed,this,&Keyboard::keyPressed);
        ofRemoveListener(ofEvents().keyReleased,this,&Keyboard::keyReleased);
        registered = false;
    }
}

rx::observable<ofKeyEventArgs> Keyboard::presses(){
    return sub_presses.get_observable();
}
rx::observable<ofKeyEventArgs> Keyboard::releases(){
    return sub_releases.get_observable();
}

//static
int Keyboard::keyFromEvent(const ofKeyEventArgs& e) {
    return e.key;
}
//static
bool Keyboard::printable(int key){
    return !(key & OF_KEY_MODIFIER) && isprint(key);
}

void Keyboard::keyPressed(ofKeyEventArgs& a){
    dest_presses.on_next(a);
}
void Keyboard::keyReleased(ofKeyEventArgs& a){
    dest_releases.on_next(a);
}


}

}