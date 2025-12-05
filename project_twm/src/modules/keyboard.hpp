#pragma once

#include <core/event.hpp>
#include <core/module.hpp>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <cstdint>
#include <functional>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// KEYBOARD MODULE - Модуль обработки клавиатуры с Xlib
////////////////////////////////////////////////////////////////////////////////
//
// Пример использования:
//   Display* display = XOpenDisplay(nullptr);
//   KeyboardModule keyboard(display);
//   
//   // Создание композитора с модулем клавиатуры
//   auto compositor = Compositor<KeyboardModule>{keyboard};
//   
//   // Установка функций публикации (композитор должен предоставить это)
//   keyboard.set_key_press_publisher([&compositor](const KeyPressEvent& e) {
//       compositor.publish(e);
//   });
//   keyboard.set_key_release_publisher([&compositor](const KeyReleaseEvent& e) {
//       compositor.publish(e);
//   });
//   
//   // Инициализация
//   compositor.initialize();
//   
//   // Главный цикл
//   CompositorRunner runner(compositor);
//   runner.run();
//   
//   compositor.cleanup();
//   XCloseDisplay(display);
//
////////////////////////////////////////////////////////////////////////////////

// Payload для событий клавиатуры
struct KeyPressPayload {
    KeySym keysym;           // Символ клавиши (XK_a, XK_Return и т.д.)
    unsigned int state;      // Модификаторы (Shift, Ctrl, Alt и т.д.)
    Window window;           // Окно, получившее событие
    Time timestamp;          // Временная метка события
    int x, y;                // Координаты указателя мыши
    unsigned int keycode;     // Код клавиши (физический код)
};

struct KeyReleasePayload {
    KeySym keysym;
    unsigned int state;
    Window window;
    Time timestamp;
    int x, y;
    unsigned int keycode;
};

// Определение событий клавиатуры
using KeyPressEvent = event<KeyPressPayload>;
using KeyReleaseEvent = event<KeyReleasePayload>;

////////////////////////////////////////////////////////////////////////////////
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ РАБОТЫ С КЛАВИАТУРОЙ
////////////////////////////////////////////////////////////////////////////////

namespace keyboard_utils {
    inline bool has_shift(unsigned int state) {
        return (state & ShiftMask) != 0;
    }

    inline bool has_control(unsigned int state) {
        return (state & ControlMask) != 0;
    }

    inline bool has_alt(unsigned int state) {
        return (state & Mod1Mask) != 0;
    }

    inline bool has_super(unsigned int state) {
        return (state & Mod4Mask) != 0;
    }

    inline bool has_modifiers(unsigned int state, unsigned int modifiers) {
        return (state & modifiers) == modifiers;
    }

    inline const char* keysym_to_string(KeySym keysym) {
        return XKeysymToString(keysym);
    }
}

class KeyboardModule : public ModuleBase<KeyboardModule> {
public:
    explicit KeyboardModule(Display* display) 
        : display_(display)
        , root_window_(DefaultRootWindow(display))
    {
        if (display_) {
            int min_keycode, max_keycode;
            XDisplayKeycodes(display_, &min_keycode, &max_keycode);
            int num_keycodes = max_keycode - min_keycode + 1;
            
            KeySym* keysyms = XGetKeyboardMapping(display_, min_keycode, 
                                                     num_keycodes, &keysyms_per_keycode_);
            if (keysyms) {
                XFree(keysyms);
            }
        }
    }

    ~KeyboardModule() {
        cleanup();
    }

    void initialize() {
        if (!display_) return;
        
        XSelectInput(display_, root_window_, 
                     KeyPressMask | KeyReleaseMask | SubstructureRedirectMask);
        
        int result = XGrabKeyboard(display_, root_window_, 
                                    False, GrabModeAsync, GrabModeAsync, CurrentTime);
        if (result != GrabSuccess) {
            std::cerr << "Warning: Failed to grab keyboard (code: " << result 
                      << "). Make sure you're running as window manager." << std::endl;
            std::cerr << "  GrabSuccess=" << GrabSuccess << ", AlreadyGrabbed=" 
                      << AlreadyGrabbed << ", GrabInvalidTime=" << GrabInvalidTime 
                      << ", GrabFrozen=" << GrabFrozen << std::endl;
        } else {
            std::cerr << "Keyboard grabbed successfully" << std::endl;
        }
        
        XSync(display_, False);
    }

    void cleanup() {
        if (display_) {
            XUngrabKeyboard(display_, CurrentTime);
            XSync(display_, False);
        }
    }

    int event_fd() const {
        if (!display_) return -1;
        return ConnectionNumber(display_);
    }

    bool handle_event() {
        if (!display_) return false;
        XEvent xevent;
        while (XPending(display_) > 0) {
            XNextEvent(display_, &xevent);

            switch (xevent.type) {
                case KeyPress:
                    process_key_press(&xevent.xkey);
                    break;
                case KeyRelease:
                    process_key_release(&xevent.xkey);
                    break;
                case MappingNotify:
                    // Обновляем маппинг клавиш при изменении раскладки
                    if (xevent.xmapping.request == MappingKeyboard) {
                        XRefreshKeyboardMapping(&xevent.xmapping);
                    }
                    break;
            }
        }

        return true;
    }

    template<auto... Bindings>
    constexpr auto register_impl(EventBus<Bindings...> bus) const {
        return bus;
    }

    template<typename PublishFunc>
    void set_key_press_publisher(PublishFunc&& publish_func) {
        key_press_publisher_ = std::forward<PublishFunc>(publish_func);
    }

    template<typename PublishFunc>
    void set_key_release_publisher(PublishFunc&& publish_func) {
        key_release_publisher_ = std::forward<PublishFunc>(publish_func);
    }

    Display* display() const { return display_; }
    Window root_window() const { return root_window_; }

private:
    void process_key_press(XKeyEvent* xkey) {
        KeySym keysym = XLookupKeysym(xkey, 0);
        
        // Отладочный вывод
        std::cerr << "Key pressed: keysym=" << keysym 
                  << ", state=" << xkey->state 
                  << ", keycode=" << xkey->keycode << std::endl;

        publish_key_press(KeyPressPayload{
            .keysym = keysym,
            .state = xkey->state,
            .window = xkey->window,
            .timestamp = xkey->time,
            .x = xkey->x,
            .y = xkey->y,
            .keycode = xkey->keycode
        });
    }

    void process_key_release(XKeyEvent* xkey) {
        KeySym keysym = XLookupKeysym(xkey, 0);

        publish_key_release(KeyReleasePayload{
            .keysym = keysym,
            .state = xkey->state,
            .window = xkey->window,
            .timestamp = xkey->time,
            .x = xkey->x,
            .y = xkey->y,
            .keycode = xkey->keycode
        });
    }

    void publish_key_press(const KeyPressPayload& payload) {
        if (key_press_publisher_) {
            KeyPressEvent event{.payload = payload};
            key_press_publisher_(event);
        }
    }

    void publish_key_release(const KeyReleasePayload& payload) {
        if (key_release_publisher_) {
            KeyReleaseEvent event{.payload = payload};
            key_release_publisher_(event);
        }
    }

    Display* display_;
    Window root_window_;
    int keysyms_per_keycode_ = 0;
    
    std::function<void(const KeyPressEvent&)> key_press_publisher_;
    std::function<void(const KeyReleaseEvent&)> key_release_publisher_;
};

