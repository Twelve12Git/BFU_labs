#include <iostream>
#include <string>
#include <core/event.hpp>

#define TEST1 true // correct cases of usage basic multiply event subscribtion and related concepts
#define TEST2 true // correct usage of command, request and event with external api(module)


#if TEST1
namespace test1 {
struct PrintMsgPayload {
    std::string msg;
};

void print_msg_handler(event<PrintMsgPayload> e) {
    std::cout << "FIRST subscription handler: " << e.payload.msg << std::endl;
}

using PrintMsgEvent = event<PrintMsgPayload>;

void another_print_msg_handler(const PrintMsgEvent& e) {
    std::cout << "SECOND subscription handler: " << e.payload.msg << std::endl;
}

auto lambda_print_msg_handler = [](PrintMsgEvent e) {
    // must't contains capture
        std::cout << "THIRD(lambda) subscription handler: " << e.payload.msg << std::endl;
    };

struct functor_print_msg_handler {
  functor_print_msg_handler(std::string &&suff) : additional_suffix(suff) {}
  void operator()(PrintMsgEvent e) const {
    std::cout << "FOURTH(functor) subscription handler: " << e.payload.msg << additional_suffix << std::endl;
  }
private:
    std::string additional_suffix;
};

void test1(){
    // creates event bus with two subscribtion on one event
    // check the concepts usage (only correct cases)
    std::cout << "-_-_-_-_-_-_-_-/ TEST 1 START \\-_-_-_-_-_-_-_-" << std::endl;

    static_assert(  Payload<PrintMsgPayload>,                                       "PrintMsgPayload is not a Payload"            );
    static_assert(  Event<event<PrintMsgPayload>>,                                  "event<PrintMsgPayload> is not an Event"      );
    static_assert(  Message<event<PrintMsgPayload>>,                                "event<PrintMsgPayload> is not an Message"    );
    static_assert(  std::is_void_v<response_t<event<PrintMsgPayload>>>,               "Wrong response_t"                            );    
    static_assert(  Handler<decltype(print_msg_handler), event<PrintMsgPayload>>,   "print_msg_handler is not a Handler"          );
    static_assert(  Handler<decltype(another_print_msg_handler), PrintMsgEvent>,    "another_print_msg_handler is not a Handler"  );
    static_assert(  Handler<decltype(lambda_print_msg_handler), PrintMsgEvent>,     "lambda_print_msg_handler is not a Handler"   );
    static_assert(  Handler<functor_print_msg_handler, PrintMsgEvent>,              "functor_print_msg_handler is not a Handler"  );
    
    constexpr auto bus = EventBus<>{}
        .subscribe<event<PrintMsgPayload>, print_msg_handler>()
        .subscribe<PrintMsgEvent, another_print_msg_handler>()
        .subscribe<PrintMsgEvent, lambda_print_msg_handler>()
        // .subscribe<PrintMsgEvent, functor_print_msg_handler(" (functor value)")>()
    ;

    const auto e = make_event<PrintMsgPayload>({"\t Event msg!"});
    bus.publish(e);
    std::cout<<"-----------------------------------------"<<std::endl;
    bus.publish(PrintMsgEvent{.payload={.msg="\t Manual created event."}});

    std::cout << "-_-_-_-_-_-_-_-\\  TEST 1 END  /-_-_-_-_-_-_-_-"  << std::endl;
}
}; // namespace
#endif

#if TEST2
namespace test2{
struct NumContainingPayload {
    int num;
};

struct GetNumPayload
{
    
};



using PrintNumEvent = event<NumContainingPayload>;
using GetNumRequest = request<GetNumPayload, int>;
using SetNumCommand = command<NumContainingPayload>;

static int external_api_var = 10;

void hndl_event(PrintNumEvent e){
    std::cout<<"Num is: " << e.payload.num << std::endl;
}
void hndl_command(SetNumCommand e){
    external_api_var = e.payload.num;
    std::cout<<"command dispatched"<<std::endl;
}
int hndl_request(GetNumRequest e){
    std::cout<<"request dispatched"<<std::endl;
    return external_api_var;
}
void test2(){
    std::cout << "-_-_-_-_-_-_-_-/ TEST 2 START \\-_-_-_-_-_-_-_-" << std::endl;


    constexpr auto bus = EventBus<>{}
        .subscribe<PrintNumEvent, hndl_event>()
        .subscribe<GetNumRequest, hndl_request>()
        .subscribe<SetNumCommand, hndl_command>()
    ;

    const auto cmd = make_command<NumContainingPayload>({42});
    bus.dispatch(cmd);

    const auto r = make_request<GetNumPayload, int>({});
    const auto new_data = bus.dispatch(r);

    const auto e = make_event<NumContainingPayload>({new_data});
    bus.publish(e);

    std::cout << "-_-_-_-_-_-_-_-\\  TEST 2 END  /-_-_-_-_-_-_-_-" << std::endl;
}
}; // namespace
#endif

int main() {
#if TEST1
    test1::test1();
#endif
#if TEST2
    test2::test2();
#endif

    return 0;
};
    