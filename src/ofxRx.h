//
//  ofxRx.h
//

#ifndef OFXRX_H
#define OFXRX_H

#include "ofMain.h"

//
// openframeworks uses some macros that are not disambiguated
//
#undef B0
#undef all_true
#undef check

#include <rxcpp/rx.hpp>

namespace ofx {

namespace rx {

namespace rx=rxcpp;
namespace rxsc=rxcpp::rxsc;

}

}

namespace ofxRx = ofx::rx;

#include "ofxRxObservableFrom.h"
#include "ofxRxMouse.h"
#include "ofxRxKeyboard.h"
#include "ofxRxUpdates.h"

#endif
