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

}

#endif
