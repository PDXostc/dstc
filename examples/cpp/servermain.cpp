#include "callbackvector.hpp"
#include "exampleserver.hpp"

CallbackVector<float, int> callback_vec_number;
CallbackVector<char[]>     callback_vec_string;

extern "C" {
    #include "dstc.h"
    DSTC_SERVER(do_number_stuff, float,, int,)
    DSTC_SERVER(do_string_stuff, char, [128])
}

void do_number_stuff(float f, int i)
{
    callback_vec_number.execute(f,  i);
}

void do_string_stuff(char* str) {
    callback_vec_string.execute(str);
}

int main() {

    // register callback via lambda, keeping a registration object
    // for RAII
    auto reg_obj = callback_vec_number.safeRegisterCallback(
        [](float f, int i) {
            std::cout << "safe callback called with " << f << " and " << i << std::endl;
        }
    );

    auto reg_id = callback_vec_string.unsafeRegisterCallback(
        [](char* str) {
            std::cout << "unsafe callback called with " << str << std::endl;
        }
    );

    // hook up an object
    ExampleServer obj(callback_vec_number, callback_vec_string);

    std::cout << "running for 10 seconds." << std::endl;
    dstc_process_events(10 * 1000 * 1000);

    // explicitly deregister at program exit
    callback_vec_string.deregisterCallback(reg_id);
}