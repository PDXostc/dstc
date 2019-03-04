#include "exampleserver.hpp"
using namespace std::placeholders; //_1, _2...

ExampleServer::ExampleServer(
    CallbackVector<float, int>& num_vector,
    CallbackVector<char[]>& str_vector)

    : _object_constructed(false),
    _num_callback_reg( num_vector.safeRegisterCallback(
        std::bind(&ExampleServer::numberFunction, this, _1, _2))
    ),
    _str_callback_reg( str_vector.safeRegisterCallback(
        std::bind(&ExampleServer::stringFunction, this, _1))
    )
{
    // constructor body
    _object_constructed = true;
}

void ExampleServer::numberFunction(float f, int i) {
    while(!_object_constructed) {} // ensure all constructor commands are done

    std::cout << "ExampleObject received " << f << " and " << i << std::endl;
}

void ExampleServer::stringFunction(char* str) {
    while(!_object_constructed) {} // ensure all constructor commands are done

    std::cout << "ExampleObject received " << str << std::endl;
}