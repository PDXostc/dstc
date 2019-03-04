#pragma once
#include "callbackvector.hpp"
#include <atomic>

class ExampleServer {
public:
    ExampleServer(
        CallbackVector<float, int>& num_vector,
        CallbackVector<char[]>& str_vector);

    void numberFunction(float f, int i);
    void stringFunction(char* str);
private:
    std::atomic<bool> _object_constructed; //!< Used to indicate function is done

    // !!! CallbackRegistration objects should be at the end of the class
    // so they go out of scope before other objects
    std::shared_ptr<CallbackRegistration> _num_callback_reg;
    std::shared_ptr<CallbackRegistration> _str_callback_reg;

};