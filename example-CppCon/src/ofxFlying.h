

struct ofxFlying : ofxCommon
{
public:
    ~ofxFlying();
    explicit ofxFlying(ofxCommon c);
    
    void setup(float guiX);
    void draw();

    ofxToggle show_text;
    ofxToggle show_window;
    ofxIntSlider selected;
    ofxLabel selectedText;
    ofxLabel flyingText;
    ofxLabel pointsInWindow;

    ofxRx::observe_source<int> selections;
    ofxRx::observe_source<bool> show_text_source;
    ofxRx::observe_source<bool> show_window_source;

    ofxPanel gui;

    std::string message;
    typedef std::tuple<ofPoint, unsigned long long> move_record;
    std::deque<move_record> move_window;
};