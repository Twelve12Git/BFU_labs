#pragma once

#include <core/event.hpp>
#include <core/module.hpp>
#include <concepts>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <cerrno>

////////////////////////////////////////////////////////////////////////////////
// COMPOSITOR/WINDOW MANAGER INTERFACE
////////////////////////////////////////////////////////////////////////////////

// Compositor - главная сущность оконного менеджера/композитора
// Управляет модулями, EventBus и жизненным циклом приложения
// 
// Архитектура:
// - Содержит центральный EventBus для связи между модулями
// - Регистрирует модули и их привязки в EventBus
// - Управляет инициализацией и очисткой модулей
// - Предоставляет интерфейс для публикации событий и отправки команд/запросов
//
// Пример использования:
//   struct InputModule : ModuleBase<InputModule> { ... };
//   struct WindowModule : ModuleBase<WindowModule> { ... };
//   
//   auto compositor = Compositor<InputModule, WindowModule>{};
//   compositor.initialize();
//   compositor.run(); // главный цикл
//   compositor.cleanup();
template<Module... Modules>
class Compositor {
public:
    using BusType = decltype(register_modules(EventBus<>{}, std::declval<Modules>()...));
    
    constexpr Compositor(Modules... modules) 
        : modules_(std::move(modules)...)
        , bus_(register_modules_impl(EventBus<>{}, std::make_index_sequence<sizeof...(Modules)>{}))
    {}

    void initialize() {
        (initialize_module<Modules>(), ...);
    }

    void cleanup() {
        (cleanup_module<Modules>(), ...);
    }

    template<Event E>
    inline void publish(const E& event) {
        bus_.template publish<E>(event);
    }

    inline template<Command C>
    void dispatch(const C& command) {
        bus_.template dispatch<C>(command);
    }

    template<Request R>
    inline auto dispatch(const R& request) -> response_t<R> {
        return bus_.template dispatch<R>(request);
    }

    constexpr const BusType& bus() const { return bus_; }
    constexpr BusType& bus() { return bus_; }

    template<Module M>
    requires (std::disjunction_v<std::is_same<M, Modules>...>)
    constexpr const M& module() const {
        return std::get<module_index_v<M>>(modules_);
    }

    template<Module M>
    requires (std::disjunction_v<std::is_same<M, Modules>...>)
    constexpr M& module() {
        return std::get<module_index_v<M>>(modules_);
    }

protected:
    template<Module M>
    void initialize_module() {
        if constexpr (InitializableModule<M>) {
            std::get<module_index_v<M>>(modules_).initialize();
        }
    }

    template<Module M>
    void cleanup_module() {
        if constexpr (CleanupModule<M>) {
            std::get<module_index_v<M>>(modules_).cleanup();
        }
    }

private:
    // Вспомогательный шаблон для получения индекса модуля в tuple
    template<Module M, Module... AllModules>
    struct module_index_helper;
    
    template<Module M, Module First, Module... Rest>
    struct module_index_helper<M, First, Rest...> {
        static constexpr std::size_t value = 
            std::is_same_v<M, First> ? 0 : 1 + module_index_helper<M, Rest...>::value;
    };
    
    template<Module M>
    struct module_index_helper<M> {
        static constexpr std::size_t value = 0; // не должно быть достигнуто
    };
    
    template<Module M>
    static constexpr std::size_t module_index_v = module_index_helper<M, Modules...>::value;

    template<auto... Bindings, std::size_t... Indices>
    constexpr auto register_modules_impl(EventBus<Bindings...> bus, std::index_sequence<Indices...>) {
        return register_modules(bus, std::get<Indices>(modules_)...);
    }

    std::tuple<Modules...> modules_;

    BusType bus_;

    template<Module... M>
    friend class CompositorRunner;
};

////////////////////////////////////////////////////////////////////////////////
// COMPOSITOR RUNNER (для управления главным циклом)
////////////////////////////////////////////////////////////////////////////////

// Концепт для модулей, которые предоставляют источник событий (файловый дескриптор)
template<typename M>
concept EventSourceModule = requires(M module) {
    { module.event_fd() } -> std::convertible_to<int>;
    { module.handle_event() } -> std::same_as<bool>; // возвращает false для остановки
};

// Концепт для модулей, которые требуют выполнения в главном цикле
template<typename M>
concept RunnableModule = requires(M module) {
    { module.run() } -> std::same_as<void>;
};

// Единый главный цикл композитора
// Использует poll() для мониторинга всех источников событий
template<Module... Modules>
class CompositorRunner {
public:
    CompositorRunner(Compositor<Modules...>& compositor) 
        : compositor_(compositor) {
        setup_event_sources();
    }

    // Запуск единого главного цикла
    // Использует poll() для мониторинга всех источников событий
    template<typename ExitCheck = std::nullptr_t>
    void run(ExitCheck exit_check = nullptr) {
        while (running_) {
            // Проверка условия выхода (если предоставлена)
            if constexpr (!std::is_same_v<ExitCheck, std::nullptr_t>) {
                if (exit_check()) {
                    running_ = false;
                    break;
                }
            }
            
            // Ожидание событий с таймаутом (для периодических задач)
            int result = poll(poll_fds_.data(), poll_fds_.size(), 100);
            
            if (result < 0) {
                // Ошибка poll
                if (errno != EINTR) {
                    break;
                }
                continue;
            }
            
            if (result == 0) {
                // Таймаут - можно выполнить периодические задачи
                continue;
            }

            // Обработка готовых файловых дескрипторов
            for (size_t i = 0; i < poll_fds_.size(); ++i) {
                if (poll_fds_[i].revents & (POLLIN | POLLPRI | POLLERR | POLLHUP)) {
                    if (!handle_event_source(i)) {
                        running_ = false;
                        break;
                    }
                }
            }
        }
    }

    // Остановка главного цикла
    void stop() {
        running_ = false;
    }

    // Проверка, работает ли главный цикл
    bool is_running() const {
        return running_;
    }

private:
    // Настройка источников событий от модулей
    void setup_event_sources() {
        (add_event_source<Modules>(), ...);
    }

    // Добавление источника событий от модуля
    template<Module M>
    void add_event_source() {
        if constexpr (EventSourceModule<M>) {
            int fd = compositor_.template module<M>().event_fd();
            if (fd >= 0) {
                poll_fds_.push_back({fd, POLLIN | POLLPRI, 0});
                event_source_indices_.push_back(module_index_v<M>);
            }
        }
    }

    // Обработка события от источника
    bool handle_event_source(size_t index) {
        if (index >= event_source_indices_.size()) {
            return true;
        }
        
        size_t module_index = event_source_indices_[index];
        return handle_module_event_by_index(module_index);
    }

    // Обработка события модуля по индексу
    bool handle_module_event_by_index(size_t target_index) {
        return handle_module_event_impl(target_index, std::make_index_sequence<sizeof...(Modules)>{});
    }

    // Реализация обработки события модуля
    template<std::size_t... Indices>
    bool handle_module_event_impl(size_t target_index, std::index_sequence<Indices...>) {
        bool result = true;
        ((Indices == target_index ? 
            (result = handle_module_event_at<Indices>(), true) : false) || ...);
        return result;
    }

    // Обработка события модуля по позиции в кортеже
    template<std::size_t Index>
    bool handle_module_event_at() {
        using ModuleType = std::tuple_element_t<Index, std::tuple<Modules...>>;
        if constexpr (EventSourceModule<ModuleType>) {
            return std::get<Index>(compositor_.modules_).handle_event();
        }
        return true;
    }

    // Вспомогательный шаблон для получения индекса модуля
    template<Module M, Module... AllModules>
    struct module_index_helper;
    
    template<Module M, Module First, Module... Rest>
    struct module_index_helper<M, First, Rest...> {
        static constexpr std::size_t value = 
            std::is_same_v<M, First> ? 0 : 1 + module_index_helper<M, Rest...>::value;
    };
    
    template<Module M>
    struct module_index_helper<M> {
        static constexpr std::size_t value = 0;
    };
    
    template<Module M>
    static constexpr std::size_t module_index_v = module_index_helper<M, Modules...>::value;

    Compositor<Modules...>& compositor_;
    std::vector<pollfd> poll_fds_;
    std::vector<size_t> event_source_indices_; // Индексы модулей для каждого fd
    bool running_ = true;
};

////////////////////////////////////////////////////////////////////////////////
// COMPOSITOR FACTORY (для создания композитора с модулями)
////////////////////////////////////////////////////////////////////////////////

// Фабрика для создания композитора с модулями
// Упрощает инициализацию и настройку
template<Module... Modules>
class CompositorFactory {
public:
    // Создание композитора с модулями по умолчанию
    static constexpr auto create() {
        return Compositor<Modules...>{Modules{}...};
    }

    // Создание композитора с пользовательскими модулями
    template<typename... ModuleArgs>
    requires (sizeof...(ModuleArgs) == sizeof...(Modules))
    static constexpr auto create(ModuleArgs&&... modules) {
        return Compositor<Modules...>{std::forward<ModuleArgs>(modules)...};
    }

    // Создание композитора с инициализацией
    template<typename... ModuleArgs>
    requires (sizeof...(ModuleArgs) == sizeof...(Modules))
    static auto create_and_initialize(ModuleArgs&&... modules) {
        auto compositor = Compositor<Modules...>{std::forward<ModuleArgs>(modules)...};
        compositor.initialize();
        return compositor;
    }
};

