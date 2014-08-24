
#pragma once


void step_two_setup(ofApp* app)
{
    auto location_points = app->locations.setup(app->show_circle).
        distinct_until_changed().
        start_with(true).
        map(
            [=](bool locations){
                if (locations) {
                    return app->mouse.
                        moves().
                        map(ofApp::pointFromMouse).
                        as_dynamic();
                } else {
                    return rxcpp::observable<>::
                        never<ofPoint>().
                        as_dynamic();
                }
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2));

    location_points.
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                app->center = c;
            });
}