

#include <ofxRx.h>

namespace ofx {

namespace rx {


Updates::~Updates() {
    clear();
}
Updates::Updates()
:
dest_updates(sub_updates.get_subscriber().as_dynamic())
{
    registered = false;
}

void Updates::setup() {
    if (!registered) {
        ofAddListener(ofEvents().update, this, &Updates::update);
        registered = true;
    }
}
void Updates::clear() {
    if (registered) {
        ofRemoveListener(ofEvents().update, this, &Updates::update);
        registered = false;
    }
}

rx::observable<ofEventArgs> Updates::events() const
{
    return sub_updates.get_observable().as_dynamic();
}

rx::observable<unsigned long long> Updates::milliseconds() const
{
    return sub_updates.
    get_observable().
    map([](const ofEventArgs&){return ofGetElapsedTimeMillis();}).
    as_dynamic();
}

rx::observable<unsigned long long> Updates::microseconds() const
{
    return sub_updates.
    get_observable().
    map([](const ofEventArgs&){return ofGetElapsedTimeMicros();}).
    as_dynamic();
}

rx::observable<float> Updates::floats() const
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

}