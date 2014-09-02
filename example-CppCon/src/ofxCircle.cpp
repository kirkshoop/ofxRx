
#include "ofApp.h"

#include <rxcpp/rx-test.hpp>
namespace rxu = rxcpp::util;
namespace rxsc = rxcpp::schedulers;

ofxCircle::~ofxCircle()
{
}

ofxCircle::ofxCircle(ofxCommon c)
    : ofxCommon(c)
    , last_period(1.0)
    , orbit_offset(0)
{
}

rxcpp::observable<float>
ofxCircle::timeInPeriodFromMilliseconds(
    rxcpp::observable<unsigned long long> timeInMilliseconds){
    return timeInMilliseconds.
        map(
            [this](unsigned long long tick){
                // map the tick into the range 0.0-1.0
                return ofMap((tick + orbit_offset) % int(orbit_period * 1000), 0, int(orbit_period * 1000), 0.0, 1.0);
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

std::string ofxCircle::test() {
    std::stringstream results;
    
    {
        auto sc = rxsc::make_test();
        auto w = sc.create_worker();
        const rxsc::test::messages<unsigned long long> m_on;
        const rxsc::test::messages<float> p_on;

        auto xs = sc.make_hot_observable({
            m_on.next(300, 250),
            m_on.next(400, 500),
            m_on.next(500, 750),
            m_on.next(600, 1000),
            m_on.completed(700)
        });

        orbit_offset = 0;
        orbit_period = 1.0;

        auto res = w.start(
           [&]() {
               return timeInPeriodFromMilliseconds(xs.skip(0));
           });

        auto required = rxu::to_vector({
            p_on.next(300, 0.25),
            p_on.next(400, 0.5),
            p_on.next(500, 0.75),
            p_on.next(600, 0.0),
            p_on.completed(700)
        });
        auto actual = res.get_observer().messages();
        
        results << "timeInPeriodFromMilliseconds: ";
        if (actual.size() == required.size() &&
            std::equal(actual.begin(), actual.end(), required.begin())) {
            results << "Passed" << std::endl;
        } else {
            results << "Failed" << std::endl;
            results << "  Required - " << required << std::endl;
            results << "  Actual   - " << actual << std::endl;
        }
    }

    {
        auto sc = rxsc::make_test();
        auto w = sc.create_worker();
        const rxsc::test::messages<float> p_on;
        const rxsc::test::messages<ofPoint> pt_on;
        
        auto roundedPt = [](ofPoint pt){
            return ofPoint(std::round(pt.x), std::round(pt.y));
        };
        
        auto xs = sc.make_hot_observable({
            p_on.next(300, 0.25),
            p_on.next(400, 0.5),
            p_on.next(500, 0.75),
            p_on.next(600, 0.0),
            p_on.completed(700)
        });
        
        orbit_radius = 50;
        
        auto res = w.start(
            [&]() {
                return orbitPointsFromTimeInPeriod(xs).map(roundedPt);
            });
        
        auto required = rxu::to_vector({
            pt_on.next(300, ofPoint(0, 50)),
            pt_on.next(400, ofPoint(-50, 0)),
            pt_on.next(500, ofPoint(0, -50)),
            pt_on.next(600, ofPoint(50, 0)),
            pt_on.completed(700)
        });
        auto actual = res.get_observer().messages();
        
        results << "orbitPointsFromTimeInPeriod: ";
        if (actual.size() == required.size() &&
            std::equal(actual.begin(), actual.end(), required.begin())) {
            results << "Passed" << std::endl;
        } else {
            results << "Failed" << std::endl;
            results << "  Required - " << required << std::endl;
            results << "  Actual   - " << actual << std::endl;
        }
    }

    return results.str();
}

void ofxCircle::setup(float guiX) {

    gui.setup();

    gui.add(run_tests.setup("run tests", false));
    gui.add(show_circle.setup("circle", true));
    gui.add(orbit_circle.setup("circle orbits", false));
    gui.add(circle_radius.setup("circle radius", 20.0, 10.0, 600.0));
    gui.add(orbit_radius.setup("orbit radius", 50.0, 10.0, 600.0));
    gui.add(orbit_period.setup("orbit period", 1.0, 0.5, 5.0));
    
    auto sources = rx::util::to_vector({window_center(), all_movement(), just_moves(), just_drags(), never()});
    auto sourcesText = std::vector<std::string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});
    
    gui.add(selected.setup("select source", 0, 0, sourcesText.size() - 1));
    gui.add(selectedText.setup("selected source", ""));

    gui.setPosition(guiX, ofGetHeight() - gui.getHeight() - 20);

    //
    // adjust offset to coorelate position across changes in period
    //
    periods.setup(orbit_period).
        distinct_until_changed().
        start_with(orbit_period).
        subscribe(
            [this](float newPeriod){
                auto tick = ofGetElapsedTimeMillis();
                auto newOffset = ofMap((tick + orbit_offset) % int(last_period * 1000), 0, int(last_period * 1000), 0, int(newPeriod * 1000));
                orbit_offset = newOffset - (tick % int(newPeriod * 1000));
                last_period = newPeriod;
            });

    //
    // enable/disable orbit calculations
    //
    auto orbit_points = orbits.setup(orbit_circle).
        distinct_until_changed().
        start_with(orbit_circle).
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

    //
    // select movement source
    //
    auto location_points = selections.setup(selected).
        distinct_until_changed().
        start_with(selected).
        map(
            [=](int locationSource){
                selectedText = sourcesText[selected % sourcesText.size()];
                return sources[selected % sources.size()];
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2)).
        as_dynamic();

    //
    // enable/disable movement
    //
    auto circle_points = locations.setup(show_circle).
        distinct_until_changed().
        start_with(show_circle).
        map(
            [=](bool show){
                return show ? location_points : never();
            }).
        switch_on_next();

    //
    // add movement and orbit
    //
    circle_points.
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

    if (run_tests) {
        ofSetColor(ofColor::white);
        ofDrawBitmapString(test(), ofPoint(10, 14, 0));
    }
    
    gui.draw();
}