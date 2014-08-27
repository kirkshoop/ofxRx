
namespace ofx {

namespace rx {

class Keyboard {
public:
    
    Keyboard();
    ~Keyboard();
    
    void setup();
    void clear();
    
    rx::observable<ofKeyEventArgs> presses();
    rx::observable<ofKeyEventArgs> releases();
    
    void keyPressed(ofKeyEventArgs& a);
    void keyReleased(ofKeyEventArgs& a);
    
    static int keyFromEvent(const ofKeyEventArgs& e);
    static bool printable(int key);
    
protected:
    bool registered;
    rx::subjects::subject<ofKeyEventArgs> sub_presses;
    rx::subscriber<ofKeyEventArgs> dest_presses;
    rx::subjects::subject<ofKeyEventArgs> sub_releases;
    rx::subscriber<ofKeyEventArgs> dest_releases;
};

}

}