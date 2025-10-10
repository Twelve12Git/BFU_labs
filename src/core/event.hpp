#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

namespace twm::event {

enum class EventStatus {
    PENDING,
    PROCESSING, 
    COMPLETED,
    FAILED,
    REJECTED
};

class EventBus;           // Forward declarations
class IEventWrapper;
class ISystemEventHandler;
class IEventSubscriber;

class ISystemEventHandler {
public:
    virtual ~ISystemEventHandler() = default;
    virtual void initialize(EventBus& bus) = 0;
    virtual void shutdown() = 0;
    virtual std::string get_name() const = 0;
    virtual void poll_events() = 0;
};

class IEventSubscriber {
public:
    virtual ~IEventSubscriber() = default;
    virtual void initialize(EventBus& bus) = 0;
    virtual void shutdown() = 0;
    virtual std::string get_name() const = 0;
};

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void handle(const IEventWrapper& event, EventBus& bus) = 0;
    virtual std::type_index get_event_type() const = 0;
};

class IEventWrapper { // type erase
public:
    virtual ~IEventWrapper() = default;
    virtual std::type_index get_type() const = 0;
    virtual EventStatus get_status() const = 0;
    virtual void set_status(EventStatus status) = 0;
    virtual std::string get_id() const = 0;
    virtual std::string get_producer() const = 0;
    virtual std::chrono::steady_clock::time_point get_timestamp() const = 0;
};

template<typename T>
struct Event {
    T payload;
    std::string id;
    std::string producer;
    EventStatus status;
    std::chrono::steady_clock::time_point timestamp;
    
    Event(std::string producer, T&& data)
        : payload(std::forward<T>(data))
        , id(generate_id())
        , producer(std::move(producer))
        , status(EventStatus::PENDING)
        , timestamp(std::chrono::steady_clock::now())
    {}
    
private:
    std::string generate_id() {
        static std::atomic<int> counter{0};
        return "event_" + std::to_string(++counter) + "_" + 
               std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    }
};

template<typename T>
class EventWrapper : public IEventWrapper {
private:
    Event<T> event_; 
public:
    EventWrapper(Event<T>&& event) 
        : event_(std::move(event))
    {}
    
    Event<T>& get_event() { return event_; }
    const Event<T>& get_event() const { return event_; }
    
    std::type_index get_type() const override {
        return std::type_index(typeid(T));
    }
    
    EventStatus get_status() const override {
        return event_.status;
    }
    
    void set_status(EventStatus status) override {
        event_.status = status;
    }
    
    std::string get_id() const override {
        return event_.id;
    }
    
    std::string get_producer() const override {
        return event_.producer;
    }
    
    std::chrono::steady_clock::time_point get_timestamp() const override {
        return event_.timestamp;
    }
};

template<typename T>
class EventHandler : public IEventHandler {
private:
    std::function<void(const T&, EventBus&)> callback_;

public:
    explicit EventHandler(std::function<void(const T&, EventBus&)> callback)
        : callback_(std::move(callback)) {}
    
    void handle(const IEventWrapper& wrapper, EventBus& bus) override {
    try {
        const auto& concrete_wrapper = dynamic_cast<const EventWrapper<T>&>(wrapper);
        callback_(concrete_wrapper.get_event().payload, bus);
    } catch (const std::bad_cast&) {
        throw std::runtime_error("EventHandler type mismatch");
    }
}
    
    std::type_index get_event_type() const override {
        return std::type_index(typeid(T));
    }
};

class EventBus {
private:
    std::unordered_map<std::type_index, std::vector<std::unique_ptr<IEventHandler>>> handlers_;
    std::vector<std::unique_ptr<ISystemEventHandler>> publishers_;
    std::vector<std::unique_ptr<IEventSubscriber>> subscribers_;
    
public:
    EventBus() = default;
    ~EventBus();
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = default;
    EventBus& operator=(EventBus&&) = default;
    
    EventBus* register_publisher(std::unique_ptr<ISystemEventHandler> publisher);
    EventBus* register_subscriber(std::unique_ptr<IEventSubscriber> subscriber);

    template<typename... Publishers>
    EventBus* register_publishers(std::unique_ptr<Publishers>... publishers) {
        (register_publisher(std::move(publishers)), ...);
        return this;
    }
     
    template<typename... Subscribers>
    EventBus* register_subscribers(std::unique_ptr<Subscribers>... subscribers) {
        (register_subscriber(std::move(subscribers)), ...);
        return this;
    }
     
    void initialize_all();
    
    void poll_all_publishers();
    
    void shutdown_all();
    
    template<typename T>
    void subscribe(std::function<void(const T&, EventBus&)> handler);
    
    template<typename T>
    EventStatus publish(Event<T>&& event);
    
    template<typename T>
    EventStatus publish(std::string producer, T&& payload);
    
    size_t get_handler_count(std::type_index type) const;
    void clear_handlers();
    void clear_handlers(std::type_index type);
    
private:
    EventStatus publish_impl(std::unique_ptr<IEventWrapper> wrapper);
};

template<typename T>
Event<T> make_event(std::string producer, T&& payload);
} // namespace 

