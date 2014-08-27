
#include "ofApp.h"


ofxHttpImage::~ofxHttpImage()
{
}

ofxHttpImage::ofxHttpImage(ofxCommon c)
    : ofxCommon(c)
    , count(0)
{
}

rxcpp::observable<ofxHttpImage::http_response_image> ofxHttpImage::http_get_image(
    rxcpp::observe_on_one_worker producer,
    int key,
    const rxcpp::observable<next_url>& urls,
    const rxcpp::observable<int> stops){
    auto requests = urls.
        map(
            [=](const next_url& url){
                return http.get(url.second).
                    subscribe_on(producer).
                    map(
                        [](const ofxRx::HttpProgress& hp){
                            auto progress = hp.
                                response().
                                map(
                                    [](ofx::HTTP::ClientResponseProgressArgs rp){
                                        if (rp.getResponse().getStatus() > 299) {
                                            throw std::runtime_error(rp.getResponse().getReason());
                                        }
                                        return rp.getProgress();
                                    }).
                                map(
                                    [](float progress){
                                        return int(progress * 100);
                                    }).
                                distinct_until_changed();
                            
                            auto data = hp.
                                body().
                                scan(
                                    std::make_shared<ofBuffer>(),
                                    [](std::shared_ptr<ofBuffer> acc, ofxRx::BufferRef<char> b){
                                        acc->append(b.begin(), b.size());
                                        return acc;
                                    }).
                                last().
                                map(
                                    [](std::shared_ptr<ofBuffer> buffy){
                                        auto pixie = std::make_shared<ofPixels>();
                                        // got all the data, do heavy lifting on the background thread
                                        ofLoadImage(*pixie, *buffy);
                                        // crop to fit in 1/10th of the window height
                                        // and 1/2 the window width
                                        auto height = std::min(ofGetHeight() / 10, pixie->getHeight());
                                        auto width = std::min(ofGetWidth() / 2, pixie->getWidth());
                                        auto newTop = std::max(0, (pixie->getHeight() / 2) - (height / 2));
                                        auto newLeft = std::max(0, (pixie->getWidth() / 2) - (width / 2));
                                        pixie->crop(newLeft, newTop, width, height);
                                        return pixie;
                                    }).
                                start_with(std::shared_ptr<ofPixels>());
                            
                            return progress.
                                combine_latest(data).
                                as_dynamic();
                        }).
                    merge().
                    observe_on(ofxRx::observe_on_update()).
                    lift<http_response_image>(
                        [=](rxcpp::subscriber<http_response_image> out){
                            return rxcpp::make_subscriber<http_response_image>(
                                [=](http_response_image progress){out.on_next(progress);},
                                [=](std::exception_ptr ex){
                                    try{std::rethrow_exception(ex);}
                                    catch(const std::exception& e) {
                                        progress_labels[key].second = e.what();
                                    }
                                    out.on_completed();
                                },
                                [=](){out.on_completed();});
                        }).
                    as_dynamic().
                    finally(
                        [=](){
                            avg[key] = (progress_labels[key].first + avg[key]) / 2;
                        }).
                    take_until(stops).
                    as_dynamic();
            });
    
    rxcpp::observable<http_response_image> flattened;
    if (key & 0x01) {
        // abort old request and start new request immediately
        flattened = requests.
            switch_on_next();
    } else {
        // hold on to new requests until the previous has finished.
        flattened = requests.
            concat();
    }
    
    return flattened.
        map(
            [=](http_response_image progress){
                int percent;
                std::shared_ptr<ofPixels> image;
                std::tie(percent, image) = progress;
                std::stringstream ss;
                ss << "avg: " << (avg[key] * 100) << "%, current: " << percent << "%";
                progress_labels[key].second = ss.str();
                progress_labels[key].first = percent / 100.0;
                if (image) {
                    progress_images[key] = ofImage(*image);
                }
                return progress;
            });
}

void ofxHttpImage::setup(float guiX) {
    
    gui.setup();
    
    gui.add(next.setup("Start Image", 0));
    gui.add(stop.setup("Stop Active", 0));
    
    const int concurrency = std::thread::hardware_concurrency();
    
    progress_labels.resize(concurrency);
    
    for (auto& label : progress_labels) {
        gui.add(label.second.setup("working", ""));
    }
    
    gui.setPosition(guiX, ofGetHeight() - gui.getHeight() - 20);

    avg.resize(concurrency);
    std::fill(avg.begin(), avg.end(), 1.0);

    progress_images.resize(concurrency);

    auto halts = stops.setup(stop);

    nexts.setup(next).
        map(
            [=](int) {
                // tracing does not support multiple threads
                trace_off();
                return rxcpp::observable<>::
                    interval(ofxRx::observe_on_update().now(), std::chrono::milliseconds(200), ofxRx::observe_on_update()).
                    take(concurrency * 2).
                    map(
                        [=](long){
                            return next_url(count++, "http://peach.blender.org/wp-content/uploads/poster_rodents_small.jpg");
                        });
            }).
        merge().
        group_by(
            [=](const next_url& nu) {
                return int(nu.first % concurrency);
            },
            [](const next_url& nu) {
                return nu;
            }).
        map(
            [=](const rxcpp::grouped_observable<int, next_url>& urls){
                auto producerthread = rxcpp::observe_on_one_worker(
                    rxcpp::observe_on_new_thread().
                        create_coordinator().
                        get_scheduler());
                return http_get_image(
                    producerthread,
                    urls.get_key(),
                    urls.skip(0),
                    halts);
            }).
        merge().
        subscribe();
}

void ofxHttpImage::draw() {
    int y = 0;
    
    ofSetColor(ofColor::white);

    for (auto& image : progress_images) {
        if (image.getHeight() > 0) {

            if (y > ofGetHeight()) {break;}
            
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - (image.getWidth() / 2), y);
            
            image.draw(0, 0, image.getWidth(), image.getHeight());

            ofPopMatrix();

            y += (image.getHeight() + 1);
        }
    }
    gui.draw();
}
