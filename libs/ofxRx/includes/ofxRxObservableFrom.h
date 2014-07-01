
namespace ofx {

namespace rx {

template<class T>
struct observe_source
{
    observe_source()
    :
    dest_t(sub_t.get_subscriber().as_dynamic())
    {
        registered = false;
    }
    
    template<class OfxSource>
    rx::observable<T> setup(OfxSource& c) {
        if (!registered) {
            c.addListener(this, &observe_source::notification);
            registered = true;
        }
        return sub_t.get_observable().as_dynamic();
    }
    
    void notification(T & t){
        dest_t.on_next(t);
    }
    
private:
    bool registered;
    rx::subjects::subject<T> sub_t;
    rx::subscriber<T> dest_t;
};

}

}
