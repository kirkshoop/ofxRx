//
//  rxofmouse.h
//

#ifndef simpleEventsExample_rxofmouse_h
#define simpleEventsExample_rxofmouse_h

#undef B0
#undef all_true
#undef check

#include <rxcpp/rx.hpp>
namespace rx=rxcpp;

#include "ofMain.h"

namespace rxof {

class Mouse {
public:

    Mouse();
    ~Mouse();
   
    void setup();
    void clear();
    
    rx::observable<ofMouseEventArgs> moves();
    rx::observable<ofMouseEventArgs> drags();
    rx::observable<ofMouseEventArgs> presses();
    rx::observable<ofMouseEventArgs> releases();

    void mouseMoved(ofMouseEventArgs & args);
    void mouseDragged(ofMouseEventArgs & args);
    void mousePressed(ofMouseEventArgs & args);
    void mouseReleased(ofMouseEventArgs & args);

protected:
    bool registered;
    rx::subjects::subject<ofMouseEventArgs> sub_moves;
    rx::observer<ofMouseEventArgs> dest_moves;
    rx::subjects::subject<ofMouseEventArgs> sub_drags;
    rx::observer<ofMouseEventArgs> dest_drags;
    rx::subjects::subject<ofMouseEventArgs> sub_presses;
    rx::observer<ofMouseEventArgs> dest_presses;
    rx::subjects::subject<ofMouseEventArgs> sub_releases;
    rx::observer<ofMouseEventArgs> dest_releases;
};

class Keyboard {
public:
    
    Keyboard();
    ~Keyboard();
    
    void setup();
    void clear();
    
    rx::observable<ofKeyEventArgs> presses();
    rx::observable<ofKeyEventArgs> releases();
    
    void keyPressed(ofKeyEventArgs& a);
    void keyReleased(ofKeyEventArgs& a);
    
protected:
    bool registered;
    rx::subjects::subject<ofKeyEventArgs> sub_presses;
    rx::observer<ofKeyEventArgs> dest_presses;
    rx::subjects::subject<ofKeyEventArgs> sub_releases;
    rx::observer<ofKeyEventArgs> dest_releases;
};

struct Updates
{
public:
    ~Updates();
    Updates();
    
    void setup();
    void clear();
    
    rx::observable<ofEventArgs> events();
    
    rx::observable<unsigned long long> milliseconds();
    
    rx::observable<unsigned long long> microseconds();
    
    rx::observable<float> floats();

    void update(ofEventArgs& a);
    
private:
    bool registered;
    rx::subjects::subject<ofEventArgs> sub_updates;
    rx::observer<ofEventArgs> dest_updates;
};

}

#endif
