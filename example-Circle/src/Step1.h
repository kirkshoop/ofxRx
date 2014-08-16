
#pragma once

void step_one_setup(ofApp* app)
{
    app->gui.add(app->show_circle.setup("circle", true));
    app->gui.add(app->circle_radius.setup("circle radius", 20.0, 10.0, 600.0));

    app->mouse.
        moves().
        map(ofApp::pointFromMouse).
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                app->center = c;
            });
}