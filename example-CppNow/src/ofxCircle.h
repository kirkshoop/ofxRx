
struct ofxCircle : ofxCommon
{
public:
    ~ofxCircle();
    explicit ofxCircle(ofxCommon c);
    
    void setup(float guiX);
    void draw();
    
    rxcpp::observable<float> timeInPeriodFromMilliseconds(rxcpp::observable<unsigned long long> timeInMilliseconds);
    rxcpp::observable<ofPoint> orbitPointsFromTimeInPeriod(rxcpp::observable<float> timeInPeriod);
    
    ofxToggle show_circle;
    ofxToggle orbit_circle;
    ofxFloatSlider circle_radius;
    ofxFloatSlider orbit_radius;
    ofxFloatSlider orbit_period;
    ofxIntSlider selected;
    ofxLabel selectedText;
    
    ofxRx::observe_source<int> selections;
    ofxRx::observe_source<bool> orbits;
    ofxRx::observe_source<bool> locations;
    ofxRx::observe_source<float> periods;
    
    float last_period;
    int orbit_offset;
    
    ofxPanel gui;
    
    ofPoint center;
};
