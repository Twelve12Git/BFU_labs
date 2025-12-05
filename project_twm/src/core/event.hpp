#pragma once

#include <type_traits>
#include <concepts>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
// MESSAGE TYPE CONCEPTS
////////////////////////////////////////////////////////////////////////////////

template<typename T>
concept Payload = std::is_trivially_copyable_v<T> || std::is_move_constructible_v<T>;

template<typename T>
concept Message = requires(T msg) {
    typename T::payload_type;
    requires Payload<typename T::payload_type>;
    { msg.payload } -> std::convertible_to<typename T::payload_type>;
};

template<typename T>
concept Event = Message<T> && !requires { typename T::response_type; };

template<typename T>
concept Command = Message<T> && !requires { typename T::response_type; };

template<typename T>
concept Request = Message<T> && requires {
    typename T::response_type;
};

////////////////////////////////////////////////////////////////////////////////
// MESSAGE TYPE TRAITS AND UTILITIES
////////////////////////////////////////////////////////////////////////////////

template<typename M>
constexpr bool has_response_type = requires { typename M::response_type; };

template<Message M>
using payload_t = typename M::payload_type;

template<typename T>
struct response_type {
    using type = void;
};

template<typename T>
requires requires { typename T::response_type; }
struct response_type<T> {
    using type = typename T::response_type;
};

template<typename T>
using response_t = typename response_type<T>::type;

////////////////////////////////////////////////////////////////////////////////
// MESSAGE TYPE IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

template<Payload P>
struct event {
    using payload_type = P;
    P payload;
};

template<Payload P>
constexpr event<P> make_event(P&& payload) {
    return event<P>{.payload = std::forward<P>(payload)};
}

template<Payload P>
struct command {
    using payload_type = P;
    P payload;
};

template<Payload P>
constexpr command<P> make_command(P&& payload) {
    return command<P>{.payload = std::forward<P>(payload)};
}

template<Payload P, typename R>
struct request {
    using payload_type = P;
    using response_type = R;
    P payload;
};

template<Payload P, typename R>
constexpr request<P, R> make_request(P&& payload) {
    return request<P, R>{.payload = std::forward<P>(payload)};
}

////////////////////////////////////////////////////////////////////////////////
// HANDLER AND SUBSCRIPTION CONCEPTS
////////////////////////////////////////////////////////////////////////////////

template<typename H, typename M>
concept Handler = (Command<M> || Request<M>)
               && std::invocable<H, M>
               && std::same_as<std::invoke_result_t<H, M>, response_t<M>>;

template<typename S, typename E>
concept Subscription = Event<E>
                    && std::invocable<S, E>
                    && std::same_as<std::invoke_result_t<S, E>, void>;

template<Message M, auto HandlerOrSubscription>
struct Binding {
    using message_type = M;
    
    constexpr auto operator()(const M& message) const -> response_t<M> {
        return HandlerOrSubscription(message);
    }
};
////////////////////////////////////////////////////////////////////////////////
// STATIC EVENT BUS
////////////////////////////////////////////////////////////////////////////////

template<auto... Bindings>
class EventBus {
public:
    template<Message M, auto H>
    constexpr auto subscribe() {
        constexpr auto new_binding = Binding<M, H>{};
        return EventBus<Bindings..., new_binding>{};
    }

    template<Event E, auto Subscription>
    constexpr auto subscribe_event() {
        constexpr auto new_binding = Binding<E, Subscription>{};
        return EventBus<Bindings..., new_binding>{};
    }

    template<Command C, auto Handler>
    constexpr auto bind_command() {
        constexpr auto new_binding = Binding<C, Handler>{};
        return EventBus<Bindings..., new_binding>{};
    }

    template<Request R, auto HandlerReq>
    constexpr auto bind_request() {
        constexpr auto new_binding = Binding<R, HandlerReq>{};
        return EventBus<Bindings..., new_binding>{};
    }

    template<Message M, auto H>
    requires (Command<M> || Request<M>)
    constexpr auto bind() {
        constexpr auto new_binding = Binding<M, H>{};
        return EventBus<Bindings..., new_binding>{};
    }

    template<Event E>
    static constexpr void publish(const E& msg) {
        (invoke_binding_if_match<Bindings, E>(msg), ...);
    }

    template<Command C>
    static constexpr void dispatch(const C& msg) {
        dispatch_impl<C, Bindings...>(msg);
    }

    template<Request R>
    static constexpr auto dispatch(const R& msg) -> response_t<R> {
        return dispatch_impl<R, Bindings...>(msg);
    }

private:
    template<auto Binding>
    using binding_message_t = typename std::decay_t<decltype(Binding)>::message_type;

    template<auto Binding, Message M>
    static constexpr void invoke_binding_if_match(const M& msg) {
        if constexpr (std::is_same_v<binding_message_t<Binding>, M>) {
            Binding(msg);
        }
    }

    template<Message M, auto First, auto... Rest>
    static constexpr auto dispatch_impl(const M& msg) -> response_t<M> {
        if constexpr (std::is_same_v<binding_message_t<First>, M>) {
            return First(msg);
        } else if constexpr (sizeof...(Rest) > 0) {
            return dispatch_impl<M, Rest...>(msg);
        } else {
            static_assert(sizeof...(Rest) >= 0, "Handler not found for message type");
            if constexpr (std::is_void_v<response_t<M>>) {
                return;
            } else {
                return response_t<M>{};
            }
        }
    }
};
