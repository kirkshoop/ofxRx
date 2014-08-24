
#include "ofApp.h"

ofxCircle::~ofxCircle()
{
}

ofxCircle::ofxCircle(ofxCommon c)
    : ofxCommon(c)
{
}

rxcpp::observable<float>
ofxCircle::timeInPeriodFromMilliseconds(
    rxcpp::observable<unsigned long long> timeInMilliseconds){
    return timeInMilliseconds.
        map(
            [this](unsigned long long tick){
                // map the tick into the range 0.0-1.0
                return ofMap(tick % int(orbit_period * 1000), 0, int(orbit_period * 1000), 0.0, 1.0);
            });
}

rxcpp::observable<ofPoint>
ofxCircle::orbitPointsFromTimeInPeriod(
    rxcpp::observable<float> timeInPeriod){
    return timeInPeriod.
        map(
            [this](float t){
                // map the time value to a point on a circle
                return ofPoint(orbit_radius * std::cos(t * 2 * 3.14), orbit_radius * std::sin(t * 2 * 3.14));
            });
}

void ofxCircle::setup() {

    gui.setup();
    
    gui.add(show_circle.setup("circle", true));
    gui.add(orbit_circle.setup("circle orbits", true));
    gui.add(circle_radius.setup("circle radius", 20.0, 10.0, 600.0));
    gui.add(orbit_radius.setup("orbit radius", 50.0, 10.0, 600.0));
    gui.add(orbit_period.setup("orbit period", 1.0, 0.5, 5.0));
    
    auto sources = rx::util::to_vector({window_center(), all_movement(), just_moves(), just_drags(), never()});
    auto sourcesText = std::vector<std::string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});
    
    gui.add(selected.setup("select source", 0, 0, sourcesText.size() - 1));
    gui.add(selectedText.setup("selected source", ""));
    
    auto orbit_points = orbits.setup(orbit_circle).
        distinct_until_changed().
        start_with(true).
        map(
            [=](bool orbits){
                if (orbits) {
                    return orbitPointsFromTimeInPeriod(
                        timeInPeriodFromMilliseconds(
                            updates().milliseconds())).
                        as_dynamic();
                } else {
                    return rxcpp::observable<>::
                        just(ofPoint(0,0)).
                        as_dynamic();
                }
            }).
        switch_on_next();
    
    auto location_points = selections.setup(selected).
        distinct_until_changed().
        start_with(0).
        map(
            [=](int locationSource){
                selectedText = sourcesText[selected % sourcesText.size()];
                return sources[selected % sources.size()];
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
    
    location_points.
        combine_latest(std::plus<>(), orbit_points).
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                center = c;
            });

}

void ofxCircle::draw() {
    ofSetColor(ofColor(0x66,0x33,0x99));
    
    if (show_circle) {
        ofCircle(center, circle_radius);
    }
    
    gui.draw();
}