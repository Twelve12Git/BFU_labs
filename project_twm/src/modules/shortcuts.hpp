#pragma once

#include <core/event.hpp>
#include <core/module.hpp>
#include <modules/keyboard.hpp>
#include <X11/keysym.h>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// SHORTCUTS MODULE - Модуль обработки шорткатов
////////////////////////////////////////////////////////////////////////////////

// Модуль обработки шорткатов
// Подписывается на события клавиатуры и обрабатывает комбинации клавиш
class ShortcutsModule : public ModuleBase<ShortcutsModule> {
public:
    ShortcutsModule() = default;

    // Регистрация модуля в EventBus
    template<auto... Bindings>
    constexpr auto register_impl(EventBus<Bindings...> bus) const {
        // Подписываемся на события клавиатуры
        return bus
            .template subscribe_event<KeyPressEvent, handle_key_press>();
    }

private:
    // Обработчик нажатия клавиши
    static void handle_key_press(const KeyPressEvent& event) {
        const auto& payload = event.payload;
        
        // Отладочный вывод
        std::cerr << "ShortcutsModule: received key press, keysym=" << payload.keysym 
                  << ", state=" << payload.state << std::endl;
        
        // Escape - завершение программы
        if (payload.keysym == XK_Escape) {
            std::cout << "Escape pressed - exiting..." << std::endl;
            // Устанавливаем флаг для остановки главного цикла
            exit_requested = true;
            return;
        }
        
        // Win+B - вывод сообщения
        // Win (Super) = Mod4Mask в X11
        // Проверяем, что нажата клавиша 'b' и зажат модификатор Mod4 (Super/Windows)
        if (payload.keysym == XK_b || payload.keysym == XK_B) {
            if (payload.state & Mod4Mask) {
                std::cout << "Win+B pressed - Hello from TWM!" << std::endl;
                return;
            }
        }
    }

    // Флаг для запроса выхода
    static inline bool exit_requested = false;

public:
    // Проверка запроса на выход
    static bool should_exit() {
        return exit_requested;
    }

    // Сброс флага (для переиспользования)
    static void reset_exit_flag() {
        exit_requested = false;
    }
};

