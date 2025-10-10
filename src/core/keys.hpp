#pragma once
#include "event.hpp"
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

namespace twm::keys {

struct KeyEvent {
    int keycode;
    bool pressed;
    uint16_t modifiers;
    std::string key_name;
    
    KeyEvent(int code, bool press, uint16_t mods = 0, std::string name = "") 
        : keycode(code), pressed(press), modifiers(mods), key_name(std::move(name)) {}
};

class KeyboardPublisher : public twm::event::ISystemEventHandler {
private:
    twm::event::EventBus* event_bus_{nullptr};
    xcb_connection_t* connection_{nullptr};
    xcb_screen_t* screen_{nullptr};
    xcb_key_symbols_t* keysyms_{nullptr};
    
    // Для глобального перехвата
    void grab_key(uint8_t keycode, uint16_t modifiers);
    
public:
    KeyboardPublisher();
    ~KeyboardPublisher();
    
    void initialize(twm::event::EventBus& bus) override;
    void shutdown() override;
    std::string get_name() const override { return "KeyboardPublisher"; }
    void poll_events() override;
};

class KeyLoggerSubscriber : public twm::event::IEventSubscriber {
private:
    twm::event::EventBus* event_bus_{nullptr};
    
public:
    void initialize(twm::event::EventBus& bus) override;
    void shutdown() override;
    std::string get_name() const override { return "KeyLoggerSubscriber"; }
    
private:
    void handle_key_event(const KeyEvent& event, twm::event::EventBus& bus);
};

} // namespace twm::keys