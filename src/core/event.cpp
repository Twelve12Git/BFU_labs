#include <core/event.hpp>
#include <iostream>

#include <core/keys.hpp> // tmp

using namespace twm::event;

EventBus::~EventBus() {
    shutdown_all();
}

EventBus* EventBus::register_publisher(std::unique_ptr<ISystemEventHandler> publisher){
    if (publisher) {
        publishers_.push_back(std::move(publisher));
    }
    return this;
};

EventBus* EventBus::register_subscriber(std::unique_ptr<IEventSubscriber> subscriber) {
    if (subscriber) {
        subscribers_.push_back(std::move(subscriber));
    }
    return this;
}

void EventBus::initialize_all() {
    std::cout << "Initializing EventBus with " 
              << publishers_.size() << " publishers and " 
              << subscribers_.size() << " subscribers" << std::endl;
    
    for (auto& subscriber : subscribers_) {
        std::cout << "Initializing subscriber: " << subscriber->get_name() << std::endl;
        subscriber->initialize(*this);
    }
    
    for (auto& publisher : publishers_) {
        std::cout << "Initializing publisher: " << publisher->get_name() << std::endl;
        publisher->initialize(*this);
    }
}

void EventBus::poll_all_publishers() {
    for (auto& publisher : publishers_) publisher->poll_events();
}

void EventBus::shutdown_all() {
    std::cout << "Shutting down EventBus..." << std::endl;
    
    for (auto& publisher : publishers_) {
        std::cout << "Shutting down publisher: " << publisher->get_name() << std::endl;
        publisher->shutdown();
    }
    
    for (auto& subscriber : subscribers_) {
        std::cout << "Shutting down subscriber: " << subscriber->get_name() << std::endl;
        subscriber->shutdown();
    }
    
    publishers_.clear();
    subscribers_.clear();
    handlers_.clear();
}

EventStatus EventBus::publish_impl(std::unique_ptr<IEventWrapper> wrapper) {
    auto type = wrapper->get_type();
    wrapper->set_status(EventStatus::PROCESSING);
    
    try {
        auto it = handlers_.find(type);
        if (it != handlers_.end() && !it->second.empty()) {
            for (auto& handler : it->second) {
                handler->handle(*wrapper, *this);
            }
            wrapper->set_status(EventStatus::COMPLETED);
        } else {
            wrapper->set_status(EventStatus::REJECTED);
        }
    } catch (const std::exception& e) {
        wrapper->set_status(EventStatus::FAILED);
    }
    
    return wrapper->get_status();
}

size_t EventBus::get_handler_count(std::type_index type) const {
    auto it = handlers_.find(type);
    return it != handlers_.end() ? it->second.size() : 0;
}

void EventBus::clear_handlers() {
    handlers_.clear();
}

void EventBus::clear_handlers(std::type_index type) {
    handlers_.erase(type);
}

template<typename T>
Event<T> make_event(std::string producer, T&& payload) {
    return Event<T>(std::move(producer), std::forward<T>(payload));
}

template<typename T>
void EventBus::subscribe(std::function<void(const T&, EventBus&)> handler) {
    auto wrapper = std::make_unique<EventHandler<T>>(std::move(handler));
    handlers_[std::type_index(typeid(T))].push_back(std::move(wrapper));
}

template<typename T>
EventStatus EventBus::publish(Event<T>&& event) {
    auto wrapper = std::make_unique<EventWrapper<T>>(std::move(event));
    return publish_impl(std::move(wrapper));
}

template<typename T>
EventStatus EventBus::publish(std::string producer, T&& payload) {
    return publish(Event<T>(std::move(producer), std::forward<T>(payload)));
}

// tmp for Keys heading linking 
template void twm::event::EventBus::subscribe<twm::keys::KeyEvent>(
    std::function<void(const twm::keys::KeyEvent&, twm::event::EventBus&)>);

template twm::event::EventStatus twm::event::EventBus::publish<twm::keys::KeyEvent>(
    twm::event::Event<twm::keys::KeyEvent>&&);

template twm::event::EventStatus twm::event::EventBus::publish<twm::keys::KeyEvent>(
    std::string, twm::keys::KeyEvent&&);
