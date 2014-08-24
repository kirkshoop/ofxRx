
namespace ofx {

namespace rx {

class Mouse {
public:

    Mouse();
    ~Mouse();
   
    void setup();
    void clear();
    
    rx::observable<ofMouseEventArgs> moves();
    rx::observable<ofMouseEventArgs> drags();
    rx::observable<ofMouseEventArgs> presses();
    rx::observable<ofMouseEventArgs> releases();

    void mouseMoved(ofMouseEventArgs & args);
    void mouseDragged(ofMouseEventArgs & args);
    void mousePressed(ofMouseEventArgs & args);
    void mouseReleased(ofMouseEventArgs & args);

    static ofPoint pointFromEvent(ofMouseEventArgs e);

protected:
    bool registered;
    rx::subjects::subject<ofMouseEventArgs> sub_moves;
    rx::subscriber<ofMouseEventArgs> dest_moves;
    rx::subjects::subject<ofMouseEventArgs> sub_drags;
    rx::subscriber<ofMouseEventArgs> dest_drags;
    rx::subjects::subject<ofMouseEventArgs> sub_presses;
    rx::subscriber<ofMouseEventArgs> dest_presses;
    rx::subjects::subject<ofMouseEventArgs> sub_releases;
    rx::subscriber<ofMouseEventArgs> dest_releases;
};

}

}