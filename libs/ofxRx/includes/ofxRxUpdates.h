
namespace ofx {

namespace rx {

struct Updates
{
    struct scheduler_type : public rxsc::scheduler_interface
    {
    private:
        typedef scheduler_type this_type;
        scheduler_type(const this_type&);

        struct worker_type : public rxsc::worker_interface
        {
        private:
            typedef worker_type this_type;

            typedef rxsc::detail::action_queue queue;

            worker_type(const this_type&);

            struct worker_state : public std::enable_shared_from_this<worker_state>
            {
                typedef rxsc::detail::schedulable_queue<
                    typename clock_type::time_point> queue_item_time;

                typedef queue_item_time::item_type item_type;

                virtual ~worker_state()
                {
                }

                explicit worker_state(rx::composite_subscription cs)
                    : lifetime(cs)
                {
                }

                rx::composite_subscription lifetime;
                mutable std::mutex lock;
                mutable queue_item_time queue;
                rxsc::recursion r;
            };

            std::shared_ptr<worker_state> state;

        public:
            virtual ~worker_type()
            {
            }

            explicit worker_type(std::shared_ptr<worker_state> ws)
                : state(ws)
            {
            }

            worker_type(rx::composite_subscription cs, rx::observable<ofEventArgs> d)
                : state(std::make_shared<worker_state>(cs))
            {
                auto keepAlive = state;

                d.subscribe(
                    state->lifetime,
                    [keepAlive](const ofEventArgs&){

                        for(;;) {
                            std::unique_lock<std::mutex> guard(keepAlive->lock);
                            if (keepAlive->queue.empty() || !keepAlive->lifetime.is_subscribed()) {
                                break;
                            }
                            auto& peek = keepAlive->queue.top();
                            if (!peek.what.is_subscribed()) {
                                keepAlive->queue.pop();
                                continue;
                            }
                            if (clock_type::now() < peek.when) {
                                break;
                            }
                            auto what = peek.what;
                            keepAlive->queue.pop();
                            keepAlive->r.reset(keepAlive->queue.empty());
                            guard.unlock();
                            what(keepAlive->r.get_recurse());
                        }
                    });
            }

            virtual clock_type::time_point now() const {
                return clock_type::now();
            }

            virtual void schedule(const rxsc::schedulable& scbl) const {
                schedule(now(), scbl);
            }

            virtual void schedule(clock_type::time_point when, const rxsc::schedulable& scbl) const {
                if (scbl.is_subscribed()) {
                    std::unique_lock<std::mutex> guard(state->lock);
                    state->queue.push(worker_state::item_type(when, scbl));
                    state->r.reset(false);
                }
            }
        };

        rx::observable<ofEventArgs> destination;

    public:
        explicit scheduler_type(rx::observable<ofEventArgs> d)
            : destination(std::move(d))
        {
        }
        virtual ~scheduler_type()
        {
        }

        virtual clock_type::time_point now() const {
            return clock_type::now();
        }

        virtual rxsc::worker create_worker(rx::composite_subscription cs) const {
            return rxsc::worker(cs, std::shared_ptr<worker_type>(new worker_type(cs, destination)));
        }
    };

public:
    ~Updates();
    Updates();
    
    void setup();
    void clear();
    
    rx::observable<ofEventArgs> events();
    
    rx::observable<unsigned long long> milliseconds();
    
    rx::observable<unsigned long long> microseconds();
    
    rx::observable<float> floats();

    void update(ofEventArgs& a);

    inline rxsc::scheduler get_scheduler() {
        static auto us = rxsc::make_scheduler<scheduler_type>(events());
        return us;
    }

    inline rx::synchronize_in_one_worker get_coordination() {
        return rx::synchronize_in_one_worker(get_scheduler());
    }
    
private:
    bool registered;
    rx::subjects::subject<ofEventArgs> sub_updates;
    rx::subscriber<ofEventArgs> dest_updates;
};

}

}