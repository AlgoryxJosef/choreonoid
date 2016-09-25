/*!
  @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_UTIL_LUA_SIGNAL_H
#define CNOID_UTIL_LUA_SIGNAL_H

#include "../Signal.h"
#include <sol.hpp>
#include <boost/type_traits.hpp>

namespace cnoid {

namespace signal_private {

template<typename T> struct lua_function_caller0 {
    sol::function func;
    lua_function_caller0(sol::function func) : func(func) { }
    T operator()() {
        T result;
        try {
            result = func();
        } catch(const sol::error& ex) {
            // use this_state
        }
        return result;
    }
};

template<> struct lua_function_caller0<void> {
    sol::function func;
    lua_function_caller0(sol::function func) : func(func) { }
    void operator()() {
        try {
            func();
        } catch(const sol::error& ex) {
            // use this_state
        }
    }
};

template<typename T, typename ARG1> struct lua_function_caller1 {
    sol::function func;
    lua_function_caller1(sol::function func) : func(func) { }
    T operator()(ARG1 arg1) {
        T result;
        try {
            result = func(arg1);
        } catch(const sol::error& ex) {
            // use this_state
        }
        return result;
    }
};

template<typename ARG1> struct lua_function_caller1<void, ARG1> {
    sol::function func;
    lua_function_caller1(sol::function func) : func(func) { }
    void operator()(ARG1 arg1) {
        try {
            func(arg1);
        } catch(const sol::error& ex) {
            // use this_state
        }
    }
};

template<typename T, typename ARG1, typename ARG2> struct lua_function_caller2 {
    sol::function func;
    lua_function_caller2(sol::function func) : func(func) { }
    T operator()(ARG1 arg1, ARG2 arg2) {
        T result;
        try {
            result = func(arg1, arg2);
        } catch(const sol::error& ex) {
            // use this_state
        }
        return result;
    }
};

template<typename ARG1, typename ARG2> struct lua_function_caller2<void, ARG1, ARG2> {
    sol::function func;
    lua_function_caller2(sol::function func) : func(func) { }
    void operator()(ARG1 arg1, ARG2 arg2) {
        try {
            func(arg1, arg2);
        } catch(const sol::error& ex) {
            // use this_state
        }
    }
};


template<int Arity, typename Signature, typename Combiner>
class lua_signal_impl;

template<typename Signature, typename Combiner>
class lua_signal_impl<0, Signature, Combiner>
{
    typedef boost::function_traits<Signature> traits;
public:
    typedef lua_function_caller0<typename traits::result_type> caller;
};

template<typename Signature, typename Combiner>
class lua_signal_impl<1, Signature, Combiner>
{
    typedef boost::function_traits<Signature> traits;
public:
    typedef lua_function_caller1<typename traits::result_type,
                                 typename traits::arg1_type> caller;
};

template<typename Signature, typename Combiner>
class lua_signal_impl<2, Signature, Combiner>
{
    typedef boost::function_traits<Signature> traits;
public:
    typedef lua_function_caller2<typename traits::result_type,
                                 typename traits::arg1_type,
                                 typename traits::arg2_type> caller;
};

} // namespace signal_private

template<
    typename Signature, 
    typename Combiner = signal_private::last_value<typename boost::function_traits<Signature>::result_type>
    >
class LuaSignalProxy : public signal_private::lua_signal_impl<
    (boost::function_traits<Signature>::arity), Signature, Combiner>
{
    typedef signal_private::lua_signal_impl<(boost::function_traits<Signature>::arity), Signature, Combiner> base_type;
    
    static Connection connect(SignalProxy<Signature, Combiner>& self, sol::function func){
        return self.connect(typename base_type::caller(func));
    }
public:
    LuaSignalProxy(const char* name, sol::table& module) {
        module.new_usertype<SignalProxy<Signature, Combiner>>(
            name, "connect", &LuaSignalProxy::connect);
    }
};


template<
    typename Signature, 
    typename Combiner = signal_private::last_value<typename boost::function_traits<Signature>::result_type>
    >
class LuaSignal : public signal_private::lua_signal_impl<
    (boost::function_traits<Signature>::arity), Signature, Combiner>
{
    typedef signal_private::lua_signal_impl<(boost::function_traits<Signature>::arity), Signature, Combiner> base_type;
    
    static Connection connect(Signal<Signature, Combiner>& self, sol::function func){
        return self.connect(typename base_type::caller(func));
    }
    static Connection connectProxy(SignalProxy<Signature, Combiner>& self, sol::function func){
        return self.connect(typename base_type::caller(func));
    }
public:
    LuaSignal(const char* name, sol::table& module) {
        module.new_usertype<Signal<Signature, Combiner>>(
            name, "connect", &LuaSignal::connect);

        module.new_usertype<SignalProxy<Signature, Combiner>>(
            (std::string(name) + "Proxy"),
            "connect", &LuaSignal::connectProxy);
    }
};

}

#endif
