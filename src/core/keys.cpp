#include "keys.hpp"
#include <iostream>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <cstring>

using namespace twm::keys;

KeyboardPublisher::KeyboardPublisher() {
    std::cout << "Connecting to X server..." << std::endl;
    connection_ = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(connection_)) {
        std::cerr << "❌ FAILED to connect to X server" << std::endl;
        connection_ = nullptr;
        return;
    }
    std::cout << "Connected to X server" << std::endl;
    
    screen_ = xcb_setup_roots_iterator(xcb_get_setup(connection_)).data;
    if (!screen_) {
        std::cerr << "❌ FAILED to get screen" << std::endl;
        xcb_disconnect(connection_);
        connection_ = nullptr;
        return;
    }
    
    keysyms_ = xcb_key_symbols_alloc(connection_);
}

KeyboardPublisher::~KeyboardPublisher() {
    shutdown();
}

void KeyboardPublisher::grab_key(uint8_t keycode, uint16_t modifiers) {
    xcb_grab_key(connection_, 1, screen_->root, modifiers, keycode, 
                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void KeyboardPublisher::initialize(twm::event::EventBus& bus) {
    if (!connection_) return;
    
    event_bus_ = &bus;
    
    std::cout << "Grabbing keys for global input..." << std::endl;
    
    uint16_t modifier_combinations[] = {
        0,  // No modifier
        XCB_MOD_MASK_SHIFT,
        XCB_MOD_MASK_CONTROL, 
        XCB_MOD_MASK_1,      // Alt
        XCB_MOD_MASK_4,      // Super/Win
        XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL,
        XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_4,
        XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_4,
        XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_4
    };
    
    for (uint8_t keycode = 8; keycode < 255; keycode++) {
        for (auto modifiers : modifier_combinations) {
            grab_key(keycode, modifiers);
        }
    }
    
    xcb_flush(connection_);
    std::cout << "Global key grabbing enabled" << std::endl;
    std::cout << "Now press any key in ANY window..." << std::endl;
}

void KeyboardPublisher::shutdown() {
    if (connection_) {
        // Отпускаем все захваченные клавиши
        xcb_ungrab_key(connection_, XCB_GRAB_ANY, screen_->root, XCB_MOD_MASK_ANY);
        xcb_flush(connection_);
        
        if (keysyms_) {
            xcb_key_symbols_free(keysyms_);
            keysyms_ = nullptr;
        }
        
        xcb_disconnect(connection_);
        connection_ = nullptr;
    }
    event_bus_ = nullptr;
}

void KeyboardPublisher::poll_events() {
    if (!connection_ || !event_bus_) return;
    
    while (xcb_generic_event_t* event = xcb_poll_for_event(connection_)) {
        uint8_t response_type = event->response_type & ~0x80;
        
        switch (response_type) {
            case XCB_KEY_PRESS: {
                xcb_key_press_event_t* key_event = (xcb_key_press_event_t*)event;
                
                std::string key_name = "Unknown";
                if (keysyms_) {
                    xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms_, key_event->detail, 0);
                    if (keysym != XCB_NO_SYMBOL) {
                        // Простое преобразование для демонстрации
                        if (keysym >= 0x20 && keysym <= 0x7E) {
                            key_name = std::string(1, (char)keysym);
                        } else {
                            switch (keysym) {
                                case 0xFFEB: case 0xFFEC: key_name = "Super"; break;
                                case 0xFFE1: case 0xFFE2: key_name = "Shift"; break;
                                case 0xFFE3: case 0xFFE4: key_name = "Control"; break;
                                case 0xFFE9: case 0xFFEA: key_name = "Alt"; break;
                                case 0xFF0D: key_name = "Enter"; break;
                                case 0xFF08: key_name = "Backspace"; break;
                                case 0xFF09: key_name = "Tab"; break;
                                case 0xFF1B: key_name = "Escape"; break;
                                case 0xFFFF: key_name = "Delete"; break;
                                default: key_name = "Key_" + std::to_string(keysym);
                            }
                        }
                    }
                }
                
                event_bus_->publish("XCB", KeyEvent(key_event->detail, true, key_event->state, key_name));
                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t* key_event = (xcb_key_release_event_t*)event;
                event_bus_->publish("XCB", KeyEvent(key_event->detail, false, key_event->state, ""));
                break;
            }
            default:
                break;
        }
        
        free(event);
    }
}

void KeyLoggerSubscriber::initialize(twm::event::EventBus& bus) {
    event_bus_ = &bus;
    
    bus.subscribe<KeyEvent>([this](const KeyEvent& event, twm::event::EventBus& bus) {
        handle_key_event(event, bus);
    });
    
    std::cout << "✅ KeyLoggerSubscriber ready for global input" << std::endl;
}

void KeyLoggerSubscriber::shutdown() {
    event_bus_ = nullptr;
}

void KeyLoggerSubscriber::handle_key_event(const KeyEvent& event, twm::event::EventBus& bus) {
    std::string mods;
    if (event.modifiers & XCB_MOD_MASK_SHIFT) mods += "Shift+";
    if (event.modifiers & XCB_MOD_MASK_CONTROL) mods += "Ctrl+";
    if (event.modifiers & XCB_MOD_MASK_1) mods += "Alt+";
    if (event.modifiers & XCB_MOD_MASK_4) mods += "Win+";
    if (event.modifiers & XCB_MOD_MASK_LOCK) mods += "Caps+";
    
    std::cout << "GLOBAL: " << (event.pressed ? "PRESS" : "RELEASE") 
              << " " << mods << event.key_name
              << " (code=" << event.keycode << ")" 
              << std::endl;
              
    if (event.pressed && (event.modifiers & XCB_MOD_MASK_4) && event.key_name == "m") {
        std::cout << "WIN+M DETECTED GLOBALLY!" << std::endl;
    }
    if (event.pressed && (event.modifiers & XCB_MOD_MASK_4) && event.key_name == "Escape") {
        std::cout << " SHUTDOWN: Win+Escape pressed - exiting..." << std::endl;
        exit(0);
    }
}