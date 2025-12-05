#include <iostream>
#include <X11/Xlib.h>
#include "core/compositor.hpp"
#include "modules/keyboard.hpp"
#include "modules/shortcuts.hpp"

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X display" << std::endl;
        return 1;
    }

    try {
        KeyboardModule keyboard(display);
        
        ShortcutsModule shortcuts;
        
        auto compositor = Compositor<KeyboardModule, ShortcutsModule>{
            keyboard, shortcuts
        };
        
        keyboard.set_key_press_publisher([&compositor](const KeyPressEvent& e) {
            compositor.publish(e);
        });
        keyboard.set_key_release_publisher([&compositor](const KeyReleaseEvent& e) {
            compositor.publish(e);
        });
        
        compositor.initialize();
        
        std::cout << "TWM started. Press Escape to exit, Win+B for message." << std::endl;
        
        CompositorRunner runner(compositor);
        
        runner.run([&]() {
            return ShortcutsModule::should_exit();
        });
        
        compositor.cleanup();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        XCloseDisplay(display);
        return 1;
    }
    
    XCloseDisplay(display);
    return 0;
}
