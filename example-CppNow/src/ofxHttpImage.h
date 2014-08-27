

struct ofxHttpImage : ofxCommon
{
public:
    ~ofxHttpImage();
    explicit ofxHttpImage(ofxCommon c);
    
    void setup(float guiX);
    void draw();

    typedef std::pair<int, std::string> next_url;
    typedef std::tuple<int, std::shared_ptr<ofPixels>> http_response_image;

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
    
    ofxRx::observe_void_source<int> nexts;
    ofxRx::observe_void_source<int> stops;
    
    ofxPanel gui;

    ofx::rx::HttpClient http;
    
    int count;

};