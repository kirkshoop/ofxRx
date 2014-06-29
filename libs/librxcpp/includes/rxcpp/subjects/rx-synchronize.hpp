// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SYNCHRONIZE_HPP)
#define RXCPP_RX_SYNCHRONIZE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace subjects {

namespace detail {

template<class T, class Coordination>
class synchronize_observer : public detail::multicast_observer<T>
{
    typedef synchronize_observer<T, Coordination> this_type;
    typedef detail::multicast_observer<T> base_type;

    typedef typename std::decay<Coordination>::type coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef typename coordinator_type::template get<subscriber<T>>::type output_type;

    struct synchronize_observer_state : public std::enable_shared_from_this<synchronize_observer_state>
    {
        typedef rxn::notification<T> notification_type;
        typedef typename notification_type::type base_notification_type;
        typedef std::deque<base_notification_type> queue_type;

        struct mode
        {
            enum type {
                Invalid = 0,
                Processing,
                Empty,
                Disposed
            };
        };

        mutable std::mutex lock;
        mutable std::condition_variable wake;
        mutable queue_type queue;
        composite_subscription lifetime;
        rxsc::worker processor;
        mutable typename mode::type current;
        coordinator_type coordinator;
        output_type destination;

        void ensure_processing(std::unique_lock<std::mutex>& guard) const {
            if (!guard.owns_lock()) {
                abort();
            }
            if (current == mode::Empty) {
                current = mode::Processing;
                auto keepAlive = this->shared_from_this();
                auto processor = coordinator.get_worker();
                processor.schedule(lifetime, [keepAlive, this](const rxsc::schedulable& self){
                    try {
                        std::unique_lock<std::mutex> guard(lock);
                        if (!destination.is_subscribed()) {
                            current = mode::Disposed;
                            queue.clear();
                            guard.unlock();
                            lifetime.unsubscribe();
                            return;
                        }
                        if (queue.empty()) {
                            current = mode::Empty;
                            return;
                        }
                        auto notification = std::move(queue.front());
                        queue.pop_front();
                        guard.unlock();
                        notification->accept(destination);
                        self();
                    } catch(...) {
                        destination.on_error(std::current_exception());
                        std::unique_lock<std::mutex> guard(lock);
                        current = mode::Empty;
                    }
                });
            }
        }

        synchronize_observer_state(coordinator_type coor, composite_subscription cs, output_type scbr)
            : lifetime(std::move(cs))
            , current(mode::Empty)
            , coordinator(std::move(coor))
            , destination(std::move(scbr))
        {
        }

        template<class V>
        void on_next(V v) const {
            if (lifetime.is_subscribed()) {
                std::unique_lock<std::mutex> guard(lock);
                queue.push_back(notification_type::on_next(std::move(v)));
                ensure_processing(guard);
            }
            wake.notify_one();
        }
        void on_error(std::exception_ptr e) const {
            if (lifetime.is_subscribed()) {
                std::unique_lock<std::mutex> guard(lock);
                queue.push_back(notification_type::on_error(e));
                ensure_processing(guard);
            }
            wake.notify_one();
        }
        void on_completed() const {
            if (lifetime.is_subscribed()) {
                std::unique_lock<std::mutex> guard(lock);
                queue.push_back(notification_type::on_completed());
                ensure_processing(guard);
            }
            wake.notify_one();
        }
    };

    std::shared_ptr<synchronize_observer_state> state;

public:
    synchronize_observer(coordination_type cn, composite_subscription dl, composite_subscription il)
        : base_type(dl)
    {
        auto o = make_subscriber<T>(dl, make_observer_dynamic<T>( *static_cast<base_type*>(this) ));

        // creates a worker whose lifetime is the same as the destination subscription
        auto coordinator = cn.create_coordinator(dl);
        auto selectedDest = on_exception(
            [&](){return coordinator.out(o);},
            o);
        if (selectedDest.empty()) {
            return;
        }

        state = std::make_shared<synchronize_observer_state>(std::move(coordinator), std::move(il), std::move(selectedDest.get()));
    }

    template<class V>
    void on_next(V v) const {
        state->on_next(std::move(v));
    }
    void on_error(std::exception_ptr e) const {
        state->on_error(e);
    }
    void on_completed() const {
        state->on_completed();
    }
};

}

template<class T, class Coordination>
class synchronize
{
    composite_subscription lifetime;
    detail::synchronize_observer<T, Coordination> s;

public:
    explicit synchronize(Coordination cn, composite_subscription cs = composite_subscription())
        : lifetime(composite_subscription())
        , s(std::move(cn), std::move(cs), lifetime)
    {
    }

    bool has_observers() const {
        return s.has_observers();
    }

    subscriber<T> get_subscriber() const {
        return make_subscriber<T>(lifetime, make_observer_dynamic<T>(observer<T, detail::synchronize_observer<T, Coordination>>(s)));
    }

    observable<T> get_observable() const {
        return make_observable_dynamic<T>([this](subscriber<T> o){
            this->s.add(std::move(o));
        });
    }
};

}

class synchronize_in_one_worker : public coordination_base
{
    rxsc::scheduler factory;

    class input_type
    {
        rxsc::worker controller;
        rxsc::scheduler factory;
        identity_one_worker coordination;
    public:
        explicit input_type(rxsc::worker w)
            : controller(w)
            , factory(rxsc::make_same_worker(w))
            , coordination(factory)
        {
        }
        rxsc::worker get_worker() const {
            return controller;
        }
        rxsc::scheduler get_scheduler() const {
            return factory;
        }
        template<class Observable>
        auto in(Observable o) const
            -> decltype(o.synchronize(coordination).ref_count()) {
            return      o.synchronize(coordination).ref_count();
        }
        template<class Subscriber>
        auto out(Subscriber s) const
            -> Subscriber {
            return std::move(s);
        }
        template<class F>
        auto act(F f) const
            -> F {
            return std::move(f);
        }
    };

public:

    explicit synchronize_in_one_worker(rxsc::scheduler sc) : factory(sc) {}

    typedef coordinator<input_type> coordinator_type;

    coordinator_type create_coordinator(composite_subscription cs = composite_subscription()) const {
        auto w = factory.create_worker(std::move(cs));
        return coordinator_type(input_type(std::move(w)));
    }
};

}

#endif