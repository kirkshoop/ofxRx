
#pragma once

struct HttpGet
{
    HttpGet(std::string u, int h);
    
    const ofx::HTTP::BaseResponse* response;
    bool done;
    bool failed;
    float progress;
    int height;
    int imageOpacity;
    std::string url;
    std::string message;
    ofBuffer data;
    ofPixels pixels;
    ofImage image;
    
    rx::observable<HttpGet*> image_from_body(ofxRx::HttpProgress& hp);
    rx::observable<HttpGet*> update_progress(ofxRx::HttpProgress& hp);
    
    rx::observable<std::tuple<ofxRx::HttpProgress, HttpGet*>> get(ofxRx::HttpClient& http);
    
    void draw(int width, int height);
};