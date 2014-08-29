

struct ofxHttpImage : ofxCommon
{
public:
    ~ofxHttpImage();
    explicit ofxHttpImage(ofxCommon c);
    
    void setup(float guiX);
    void draw();

    typedef std::pair<int, std::string> next_url;
    typedef std::tuple<int, std::shared_ptr<ofPixels>> http_response_image;

    static
    rxcpp::observable<next_url>
    send_urls(int);

    static
    ofx::HTTP::ClientResponseProgressArgs
    http_status_to_error(ofx::HTTP::ClientResponseProgressArgs rp);

    static
    rxcpp::observable<int>
    http_progress(const ofxRx::HttpProgress& hp);

    static
    std::shared_ptr<ofPixels>
    image_from_buffer(std::shared_ptr<ofBuffer> buffy);

    static
    rxcpp::observable<std::shared_ptr<ofPixels>>
    http_image(const ofxRx::HttpProgress& hp);

    static
    rxcpp::observable<http_response_image>
    http_progress_image(const ofxRx::HttpProgress& hp);

    rxcpp::observable<http_response_image> make_http_request(
        rxcpp::observe_on_one_worker producer,
        int key,
        const next_url& url,
        const rxcpp::observable<int> stops);

    rxcpp::subscriber<http_response_image>
    error_display(int key, rxcpp::subscriber<http_response_image> out);

    http_response_image
    update_ui(int key, http_response_image progress);

    rxcpp::observable<http_response_image> http_get_image(
        rxcpp::observe_on_one_worker producer,
        int key,
        const rxcpp::observable<next_url>& urls,
        const rxcpp::observable<int> stops);
    
    std::vector<float> avg;
    std::vector<std::pair<float, ofxLabel>> progress_labels;
    std::vector<ofImage> progress_images;
    
    ofxButton next;
    ofxButton stop;
    ofxToggle show_image;
    
    ofxRx::observe_void_source<int> nexts;
    ofxRx::observe_void_source<int> stops;
    
    ofxPanel gui;

    ofx::rx::HttpClient http;
    
    int queued;

};