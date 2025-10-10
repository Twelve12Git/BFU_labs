#include <core/event.hpp>
#include <core/keys.hpp>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

volatile bool running = true;

void signal_handler(int) {
    std::cout << "\nShutting down..." << std::endl;
    running = false;
}

int main() {
    using namespace twm;
    
    std::signal(SIGINT, signal_handler);
    
    std::cout << "=== XCB Event System Test ===" << std::endl;
    
    event::EventBus bus;
    
    
    bus.register_publishers(  
            std::make_unique<keys::KeyboardPublisher>()
        )->register_subscribers(
            std::make_unique<keys::KeyLoggerSubscriber>()
        )->initialize_all();

    std::cout << "System initialized successfully!" << std::endl;
    std::cout << "Focus the 'Event Test Window' and press keys..." << std::endl;

    int iteration = 0;
    while (running) {
        bus.poll_all_publishers();
        
        iteration++;
        if (iteration % 500 == 0) {
            std::cout << "Still running... (" << iteration << " iterations)" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}