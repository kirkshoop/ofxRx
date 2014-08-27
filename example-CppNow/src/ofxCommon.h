
struct ofxCommon
{
    ~ofxCommon();
    ofxCommon();
    
    struct type
    {
        ~type();
        type();
        
        ofxRx::Mouse mouse;
        ofxRx::Keyboard keyboard;
        ofxRx::Updates updates;
        
#if RXCPP_VIEW_TRACE
        ofxRxTrace& vt;
#endif
    };
    
    std::shared_ptr<type> shared;
    
    inline ofxRx::Mouse& mouse() {return shared->mouse;}
    inline ofxRx::Keyboard& keyboard() {return shared->keyboard;}
    inline ofxRx::Updates& updates() {return shared->updates;}

    rx::observable<ofPoint> window_center();
    rx::observable<ofPoint> all_movement();
    rx::observable<ofPoint> just_moves();
    rx::observable<ofPoint> just_drags();
    rx::observable<ofPoint> never();

    void trace_off();
    
    void setup();
    void draw();
};