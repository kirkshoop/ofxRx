
#pragma once


void step_four_setup(ofApp* app)
{
    auto window_center = rx::observable<>::
        defer(
            [](){
                return rx::observable<>::
                    just(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
            }).
        as_dynamic();
    
    auto all_movement = rx::observable<>::
        from(app->mouse.moves(), app->mouse.drags()).
        merge().
        map(ofApp::pointFromMouse).
        as_dynamic();
    
    auto just_moves = app->mouse.
        moves().
        map(ofApp::pointFromMouse).
        as_dynamic();
    
    auto just_drags = app->mouse.
        drags().
        map(ofApp::pointFromMouse).
        as_dynamic();
    
    auto never = rx::observable<>::
        never<ofPoint>().
        as_dynamic();

    auto sources = rx::util::to_vector({window_center, all_movement, just_moves, just_drags, never});
    auto sourcesText = std::vector<std::string>({"window_center", "all_movement", "just_moves", "just_drags", "never"});

    app->gui.add(app->selected.setup("select source", 0, 0, sourcesText.size() - 1));
    app->gui.add(app->selectedText.setup("selected source", ""));

    auto orbit_points = app->orbits.setup(app->orbit_circle).
        distinct_until_changed().
        start_with(true).
        map(
            [=](bool orbits){
                if (orbits) {
                    return app->orbitPointsFromTimeInPeriod(
                        app->timeInPeriodFromMilliseconds(
                            app->updates.milliseconds())).
                        as_dynamic();
                } else {
                    return rxcpp::observable<>::
                        just(ofPoint(0,0)).
                        as_dynamic();
                }
            }).
        switch_on_next();
    
    auto location_points = app->selections.setup(app->selected).
        distinct_until_changed().
        start_with(0).
        map(
            [=](int locationSource){
                app->selectedText = sourcesText[app->selected % sourcesText.size()];
                return sources[app->selected % sources.size()];
            }).
        switch_on_next().
        start_with(ofPoint(ofGetWidth()/2, ofGetHeight()/2));
    
    location_points.
        combine_latest(std::plus<>(), orbit_points).
        subscribe(
            [=](ofPoint c){
                // update the point that the draw() call will use
                app->center = c;
            });
}
