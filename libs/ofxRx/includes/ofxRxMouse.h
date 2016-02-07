
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
    rx::observable<ofMouseEventArgs> scrolls();
    rx::observable<ofMouseEventArgs> enters();
    rx::observable<ofMouseEventArgs> exits();

    void mouseMoved(ofMouseEventArgs & args);
    void mouseDragged(ofMouseEventArgs & args);
    void mousePressed(ofMouseEventArgs & args);
    void mouseReleased(ofMouseEventArgs & args);
    void mouseScrolled(ofMouseEventArgs & args);
    void mouseEntered(ofMouseEventArgs & args);
    void mouseExited(ofMouseEventArgs & args);

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
    rx::subjects::subject<ofMouseEventArgs> sub_scrolls;
    rx::subscriber<ofMouseEventArgs> dest_scrolls;
    rx::subjects::subject<ofMouseEventArgs> sub_enters;
    rx::subscriber<ofMouseEventArgs> dest_enters;
    rx::subjects::subject<ofMouseEventArgs> sub_exits;
    rx::subscriber<ofMouseEventArgs> dest_exits;
};

}

}