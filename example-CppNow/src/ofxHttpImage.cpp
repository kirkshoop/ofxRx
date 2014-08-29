
#include "ofApp.h"


ofxHttpImage::~ofxHttpImage()
{
}

ofxHttpImage::ofxHttpImage(ofxCommon c)
    : ofxCommon(c)
    , queued(0)
{
}

//static
rxcpp::observable<ofxHttpImage::next_url>
ofxHttpImage::send_urls(int) {
    static int count = 0;
    // adds the image url multiple times (20)
    // one url is added every 200 milliseconds
    return rxcpp::observable<>::
        interval(ofxRx::observe_on_update().now(), std::chrono::milliseconds(200), ofxRx::observe_on_update()).
        take(20).
        map(
            [=](long){
                return next_url(count++, "http://peach.blender.org/wp-content/uploads/poster_rodents_small.jpg");
            });
}

//static
ofx::HTTP::ClientResponseProgressArgs
ofxHttpImage::http_status_to_error(ofx::HTTP::ClientResponseProgressArgs rp){
    // convert failure response codes to error
    if (rp.getResponse().getStatus() > 299) {
        throw std::runtime_error(rp.getResponse().getReason());
    }
    return rp;
}

//static
rxcpp::observable<int>
ofxHttpImage::http_progress(const ofxRx::HttpProgress& hp) {
    return hp.
        response().
        map(http_status_to_error).
        map(
            [](ofx::HTTP::ClientResponseProgressArgs rp){
                return int(rp.getProgress() * 100);
            }).
        distinct_until_changed();
}

//static
std::shared_ptr<ofPixels>
ofxHttpImage::image_from_buffer(std::shared_ptr<ofBuffer> buffy){
    auto pixie = std::make_shared<ofPixels>();
    ofLoadImage(*pixie, *buffy);
    // crop to fit in 1/10th of the window height
    // and 1/2 the window width
    auto height = std::min(ofGetHeight() / 10, pixie->getHeight());
    auto width = std::min(ofGetWidth() / 2, pixie->getWidth());
    auto newTop = std::max(0, (pixie->getHeight() / 2) - (height / 2));
    auto newLeft = std::max(0, (pixie->getWidth() / 2) - (width / 2));
    pixie->crop(newLeft, newTop, width, height);
    return pixie;
}

//static
rxcpp::observable<std::shared_ptr<ofPixels>>
ofxHttpImage::http_image(const ofxRx::HttpProgress& hp) {
    return hp.
        body().
        scan(
             std::make_shared<ofBuffer>(),
             [](std::shared_ptr<ofBuffer> acc, ofxRx::BufferRef<char> b){
                 acc->append(b.begin(), b.size());
                 return acc;
             }).
        last().
        // got all the data, do heavy lifting on the background thread
        map(image_from_buffer).
        as_dynamic();
}

//static
rxcpp::observable<ofxHttpImage::http_response_image>
ofxHttpImage::http_progress_image(const ofxRx::HttpProgress& hp) {
    return http_progress(hp).
        combine_latest(
            http_image(hp).
                start_with(std::shared_ptr<ofPixels>())).
        as_dynamic();
}

rxcpp::subscriber<ofxHttpImage::http_response_image>
ofxHttpImage::error_display(int key, rxcpp::subscriber<http_response_image> out){
    return rxcpp::make_subscriber<http_response_image>(
        [=](http_response_image progress){out.on_next(progress);},
        [=](std::exception_ptr ex){
            try{std::rethrow_exception(ex);}
            catch(const std::exception& e) {
                progress_labels[key].second = e.what();
            }
            out.on_error(ex);
        },
        [=](){out.on_completed();}).
        as_dynamic();
}

rxcpp::observable<ofxHttpImage::http_response_image>
ofxHttpImage::make_http_request(
    rxcpp::observe_on_one_worker producer,
    int key,
    const next_url& url,
    const rxcpp::observable<int> stops){
    
    ++queued;
    // tracing does not support multiple threads
    trace_off();
    
    return http.get(url.second).
        subscribe_on(producer).
        map(http_progress_image).
        merge().
        observe_on(ofxRx::observe_on_update()).
        lift<http_response_image>(
            [=](rxcpp::subscriber<http_response_image> out){
                return error_display(key, out);
            }).
        as_dynamic().
        finally(
            [=](){
                if (--queued == 0) {
                    trace_on();
                }
                avg[key] = (progress_labels[key].first + avg[key]) / 2;
            }).
        retry().
        take_until(stops).
        as_dynamic();
}

ofxHttpImage::http_response_image
ofxHttpImage::update_ui(int key, http_response_image progress){
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
}
    
rxcpp::observable<ofxHttpImage::http_response_image>
ofxHttpImage::http_get_image(
    rxcpp::observe_on_one_worker producer,
    int key,
    const rxcpp::observable<next_url>& urls,
    const rxcpp::observable<int> stops){

    rxcpp::observable<http_response_image> flattened;
    if (key & 0x01) {
        return urls.
            map(
                [=](const next_url& url){
                    return make_http_request(producer, key, url, stops);
                }).
            // abort old request and start new request immediately
            switch_on_next().
            map(
                [=](http_response_image progress){
                    return update_ui(key, progress);
                });
    } else {
        return urls.
            map(
                [=](const next_url& url){
                    return make_http_request(producer, key, url, stops);
                }).
            // hold on to new requests until the previous has finished.
            concat().
            map(
                [=](http_response_image progress){
                    return update_ui(key, progress);
                });
    }
}

void ofxHttpImage::setup(float guiX) {
    
    gui.setup();
    
    gui.add(next.setup("Start Image", 0));
    gui.add(stop.setup("Stop Active", 0));
    gui.add(show_image.setup("show images", true));
    
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

    // clear images when stopped
    halts.
        subscribe(
            [this](int){
                for(auto& image : progress_images) {
                    image.clear();
                }
            });

    nexts.setup(next).
        map(send_urls).
        merge().
        group_by(
            [=](const next_url& nu) {
                // round-robin requests across multiple threads
                return int(nu.first % concurrency);
            },
            [](const next_url& nu) {return nu;}).
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
    if (show_image) {
        int y = 0;
        
        ofSetColor(ofColor::white);

        for (auto& image : progress_images) {
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - (image.getWidth() / 2), y);
            if (image.getHeight() > 0) {

                if (y > ofGetHeight()) {break;}
                
                image.draw(0, 0, image.getWidth(), image.getHeight());

                y += image.getHeight();
            } else {
                ofDrawBitmapString("Image is empty", ofPoint(10, 14, 0));
                y += 14;
            }
            ofPopMatrix();
        }
    }

    gui.draw();
}
