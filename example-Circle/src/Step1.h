
#pragma once

void step_one_setup(ofApp* app)
{
    auto orbit_points = app->orbitPointsFromTimeInPeriod(
        app->timeInPeriodFromMilliseconds(
            app->updates.
                milliseconds()));

    auto location_points = app->mouse.
        moves().
        map(ofApp::pointFromMouse);
    
    location_points.
        combine_latest(std::plus<>(), orbit_points).
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                app->center = c;
            });
}