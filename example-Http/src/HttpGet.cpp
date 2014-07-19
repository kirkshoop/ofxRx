
#include "ofApp.h"

HttpGet::HttpGet(std::string u, int h):
    response(nullptr),
    done(false),
    failed(false),
    height(h),
    imageOpacity(255),
    url(std::move(u)),
    progress(0.0),
    message("pending...")
{
}

rx::observable<HttpGet*> HttpGet::image_from_body(ofxRx::HttpProgress& hp){
    return hp.body().
        reduce(this,
            [](HttpGet* hg, ofxRx::BufferRef<char> br){
                if (!hg->response) {
                    hg->response = std::addressof(br.getResponse());
                }
                hg->data.append(br.begin(), br.size());
                return hg;
            },
            [](HttpGet* hg){
                // got all the data, do heavy lifting on the background thread
                ofLoadImage(hg->pixels, hg->data);
                // crop to 100px of the middle
                auto newTop = std::max(0, (hg->pixels.getHeight() / 2) - (hg->height / 2));
                hg->pixels.crop(0, newTop, hg->pixels.getWidth(), hg->height);
                return hg;
            }
        ).
        as_dynamic().
        
        // update image ui from the ofupdate event
        observe_on(ofx::rx::synchronize_update()).
        as_dynamic().
        map([](HttpGet* hg){
            hg->image = ofImage(hg->pixels);
            return hg;
        }).
        as_dynamic();
}

rx::observable<HttpGet*> HttpGet::update_progress(ofxRx::HttpProgress& hp){
    return hp.response().
    map([](ofx::HTTP::ClientResponseProgressArgs rp){
        return rp.getProgress();
    }).
    as_dynamic().
    
    // update progress ui from the ofupdate event
    observe_on(ofx::rx::synchronize_update()).
    as_dynamic().
    map([=](float p){
        progress = p;
        return this;
    }).
    as_dynamic();
}

rx::observable<std::tuple<ofxRx::HttpProgress, HttpGet*>> HttpGet::get(ofxRx::HttpClient& http) {
    return http.get(url).
        flat_map(
             [=](ofxRx::HttpProgress hp){
                 
                 auto merged = image_from_body(hp).
                 merge(update_progress(hp)).
                 publish();
                 
                 auto request_failed = merged.
                 filter([=](HttpGet* hg){
                     if (hg->response) {
                         // trigger take_until to stop the request if the
                         // server status is a failure
                         hg->failed = hg->done = (hg->response->getStatus() > 299);
                         return hg->failed;
                     }
                     return false;
                 });
                 
                 // track errors and completion for the ui
                 merged.
                     take_until(request_failed).
                     subscribe(
                        [](HttpGet*){},
                        [=](std::exception_ptr ex){
                           try {
                               std::rethrow_exception(ex);
                           } catch(const std::exception& e) {
                               failed = done = true;
                               message = e.what();
                           }
                        },
                        [=](){
                           if (response) {
                               message = response->getReason();
                           } else {
                               message = "completed";
                           }
                           response = nullptr;
                           done = true;
                        });

                 // this makes the flat_maps produce no values, but
                 // only complete when the url subject is completed and
                 // all the pending responses have completed.
                return merged.
                    ref_count().
                    lift([](rx::subscriber<HttpGet*> dest){
                        return rx::make_subscriber<HttpGet*>(
                            // filter out all on_next
                            [](HttpGet*){},
                            // an error should not stop other requests
                            [=](std::exception_ptr ex){dest.on_completed();},
                            [=](){dest.on_completed();}
                        );
                    }).
                    as_dynamic();
             },
             rx::util::pack()
         ).
    
        // use event loop thread to run the http request
        subscribe_on(rx::serialize_event_loop());
}

void HttpGet::draw(int width, int height) {
    
    if (!done) {
        ofSetColor(0, 255, 0, 50);
    } else if (!failed) {
        ofSetColor(255, 255, 255, imageOpacity);
    } else {
        ofSetColor(255, 0, 0);
    }
    
    if (progress >= 1.0 && !failed) {
        image.draw(0, 0, width, height);
    } else {
        ofRect(0, 0, width, height);
        
        if (progress > 0)
        {
            ofFill();
            ofSetColor(255, 255, 0, 75);
            ofRect(0, 0, progress * width, height);
        }
    }
    
    ofSetColor(255);
    
    std::stringstream ss;
    
    ss << url + " " << (progress * 100) << "%: " << message;
    
    ofDrawBitmapString(ss.str(), ofPoint(10, 14, 0));
}

