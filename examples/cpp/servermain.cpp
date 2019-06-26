#include "callbackvector.hpp"
#include "exampleserver.hpp"

CallbackVector<float, int> callback_vec_number;
CallbackVector<char[]>     callback_vec_string;

extern "C" {
    #include "dstc.h"
    DSTC_SERVER(do_number_stuff, float,, int,)
    DSTC_SERVER(do_string_stuff, char, [128])
}

// Define DSTC function to be called by the C++ wrapper
void do_number_stuff(float f, int i)
{
    callback_vec_number.execute(f,  i);
}

// Define DSTC function to be called by the C++ wrapper
void do_string_stuff(char* str) {
    callback_vec_string.execute(str);
}

int main() {

    // register callback via lambda, keeping a registration object
    // for RAII
    auto reg_obj = callback_vec_number.safeRegisterCallback(
        [](float f, int i) {
            std::cout << "function callback called with " << f << " and " << i << std::endl;
        }
    );

    // hook up callbacks via an object (see ExampleServer::ExampleServer())
    ExampleServer obj(callback_vec_number, callback_vec_string);

    std::cout << "running for 10 seconds." << std::endl;
    dstc_msec_timestamp_t current_ts = dstc_msec_monotonic_timestamp();
    dstc-msec_timestamp_t stop_ts = current_ts + 10000;
    while(current_ts < stop_ts) {
        dstc_process_events(stop_ts - current_ts);
        current_ts = dstc_msec_monotonic_timestamp();
    }
    exit(0);
}
