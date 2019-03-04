#pragma once

#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <memory>

/*!
 * This object will hold onto a callback registration until this object
 * goes out of scope.  It is used to allow callbacks to be deregistered
 * with the RAII idiom in mind.
 * This object gets returned by CallbackVector::safeRegisterCallback
 */
class CallbackRegistration {
public:
    /*!
     * Creates a CallbackRegistration object
     * \param deregisterFunction Function that gets executed when object is destroyed
     */
    CallbackRegistration(std::function<void()> deregisterFunction)
    : _deregister_function(deregisterFunction) {}
    ~CallbackRegistration() {
        if (_deregister_function)
            _deregister_function();
        }
    // make uncopyable
    CallbackRegistration(const CallbackRegistration&) = delete;

private:
    std::function<void()> _deregister_function; //!< Deregistration function
};

/*!
 * This object allows registration of function callbacks to be executed when
 * execute() gets called
 *
 * /tparam TYPES parameters accepted by function call (example, if this
 * registers functions with the parameter void(float i, char[3]), this
 * would be created as CallbackVector<float, char[]>
 */
template <class ... TYPES>
class CallbackVector {
public:
    CallbackVector() : _next_id(0) {}

    typedef std::function<void( TYPES ... )> Callback_t;
    typedef unsigned int CallbackID_t;

    /*!
     * Registers a callback, and gets a CallbackRegistration object back
     * \param function Function to be invoked when CallbackVector::execute is called
     * \returns CallbackRegistration Object that keeps function registered -- when it goes out of scope your function is automatically deregistered
     *
     * \warning You must hold on to the CallbackRegistration item -- when it goes
     * out of scope, your function will immediately be deregistered
     */

    std::shared_ptr<CallbackRegistration> safeRegisterCallback(Callback_t function) {
        std::lock_guard<std::mutex> lock(_mtx);
        return std::make_shared<CallbackRegistration>(
            std::bind(&CallbackVector< TYPES ... >::deregisterCallback, this, _registerAndGetID(function))
        );
    }

    /*!
     * Register a callback
     *
     * \param function Function to be called when CallbackVector::execute is called
     * \returns CallbackID_t ID to deregister
     *
     * \warning You must explicitly call deregisterCallback to stop callbacks!
     */
    CallbackID_t unsafeRegisterCallback(Callback_t function) {
        std::lock_guard<std::mutex> lock(_mtx);
        return _registerAndGetID(function);
    }

    /*!
     * Deregisters a callback.  If you use the safe version of register, you do not
     * need to call this explicitly
     * \param id ID to deregister
     */
    void deregisterCallback(CallbackID_t id) {
        std::lock_guard<std::mutex> lock(_mtx);
        std::cout << "Deregistering callback function ID " << id << std::endl;
        _callbacks.erase(id);
    }

    /*!
     * Invokes every registered callback.  Order of callbacks is not guaranteed
     * \param args Args to pass to callback (see template definition)
     */
    void execute( TYPES ... args ) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_callbacks.empty()) {
            std::cout << "Warning -- callback happened but no callbacks registered" << std::endl;
        }

        for (auto& callback : _callbacks) {
            callback.second(args...);
        }
    }

private:

    CallbackID_t _registerAndGetID(Callback_t function) {
        auto id = _getNextID();
        _callbacks[id] = function;
        std::cout << "Registered callback function ID " << id << std::endl;
        return id;
    }

    CallbackID_t _getNextID() {
        while( (_callbacks.find(++_next_id) != _callbacks.end() ) ) ;
        return _next_id;
    }

private:
    std::unordered_map<CallbackID_t, Callback_t> _callbacks;  //!< Storage for callback
    CallbackID_t _next_id;                                    //!< Next ID to give out
    std::mutex   _mtx;                                        //!< Lock to make this thread-safe
};