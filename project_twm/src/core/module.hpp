#pragma once

#include <core/event.hpp>
#include <concepts>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////
// MODULE CONCEPT AND INTERFACE
////////////////////////////////////////////////////////////////////////////////

template<typename P, typename E>
concept Publisher = Event<E> 
                 && std::invocable<P> 
                 && std::same_as<std::invoke_result_t<P>, E>;

template<typename M>
concept Module = requires(M module) {

    { module.template register_in<>(EventBus<>{}) };
};

////////////////////////////////////////////////////////////////////////////////
// MODULE BASE STRUCTURE
////////////////////////////////////////////////////////////////////////////////

// Базовый интерфейс модуля
// Модуль определяет:
// - События (Events), которые он публикует через Publisher'ы
// - Команды (Commands), которые он обрабатывает через Handler'ы
// - Запросы (Requests), которые он обрабатывает через Handler'ы
// - Подписки (Subscriptions) на события других модулей
//
// Пример использования:
//   struct MyModule : ModuleBase<MyModule> {
//       // Определение обработчиков и подписок как статических функций или лямбд
//       static void handle_command(const MyCommand& cmd) { ... }
//       static void handle_request(const MyRequest& req) -> Response { ... }
//       static void on_event(const SomeEvent& evt) { ... }
//       
//       // Реализация регистрации модуля
//       template<auto... Bindings>
//       constexpr auto register_impl(EventBus<Bindings...> bus) const {
//           return bus
//               .template bind_command<MyCommand, handle_command>()
//               .template bind_request<MyRequest, handle_request>()
//               .template subscribe_event<SomeEvent, on_event>();
//       }
//   };
//
//   // Регистрация модуля в EventBus
//   constexpr auto bus = register_module(EventBus<>{}, MyModule{});
template<typename Derived>
struct ModuleBase {
    // Метод регистрации модуля в EventBus
    // Должен быть переопределен в производных классах
    template<auto... Bindings>
    constexpr auto register_in(EventBus<Bindings...> bus) const {
        return static_cast<const Derived*>(this)->register_impl(bus);
    }

protected:
    template<auto... Bindings, Event E, auto Subscription>
    constexpr auto subscribe_event(EventBus<Bindings...> bus) const {
        return bus.template subscribe_event<E, Subscription>();
    }

    template<auto... Bindings, Command C, auto Handler>
    constexpr auto bind_command(EventBus<Bindings...> bus) const {
        return bus.template bind_command<C, Handler>();
    }

    template<auto... Bindings, Request R, auto Handler>
    constexpr auto bind_request(EventBus<Bindings...> bus) const {
        return bus.template bind_request<R, Handler>();
    }
};

////////////////////////////////////////////////////////////////////////////////
// MODULE REGISTRATION UTILITIES
////////////////////////////////////////////////////////////////////////////////

// Вспомогательная функция для регистрации модуля в EventBus
// Позволяет использовать модуль как инъекцию зависимости
template<auto... Bindings, typename M>
constexpr auto register_module(EventBus<Bindings...> bus, const M& module) {
    return module.register_in(bus);
}

template<auto... Bindings, typename M, typename... Modules>
constexpr auto register_modules(EventBus<Bindings...> bus, const M& module, const Modules&... modules) {
    if constexpr (sizeof...(modules) == 0) {
        return register_module(bus, module);
    } else {
        return register_modules(register_module(bus, module), modules...);
    }
}

////////////////////////////////////////////////////////////////////////////////
// MODULE LIFECYCLE
////////////////////////////////////////////////////////////////////////////////

template<typename M>
concept InitializableModule = requires(M module) {
    { module.initialize() } -> std::same_as<void>;
};

template<typename M>
concept CleanupModule = requires(M module) {
    { module.cleanup() } -> std::same_as<void>;
};
