

struct ofxFlying : ofxCommon
{
public:
    ~ofxFlying();
    explicit ofxFlying(ofxCommon c);
    
    void setup();
    void draw();

    ofxToggle show_text;
    ofxToggle show_window;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    ofxLabel pointsInWindow;

    ofxRx::observe_source<int> selections;

    ofxPanel gui;

    rx::subjects::subject<rx::observable<ofPoint>> center_source;
    rx::subscriber<rx::observable<ofPoint>> dest_center;
    std::string message;
    typedef std::tuple<ofPoint, unsigned long long> move_record;
    std::deque<move_record> move_window;

};