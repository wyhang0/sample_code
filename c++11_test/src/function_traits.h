//
// Created by root on 7/30/21.
//

#ifndef C__11_TEST_FUNCTION_TRAITS_H
#define C__11_TEST_FUNCTION_TRAITS_H

#include <tuple>
#include <functional>

using namespace std;

// 类模板
template<typename Fun>
struct function_traits;

// 普通函数特化
template<typename R, typename ...Args>
struct function_traits<R(Args...)>{
    template<size_t Index>
    struct args{
        static_assert(Index < sizeof...(Args), "index out of range");
        using type = typename tuple_element<Index, tuple<Args...>>::type;
    };
    // 参数个数
    enum {arity = sizeof...(Args)};
    // 函数类型
    using function_type = R(Args...);
    // function类型
    using stl_function = function<function_type>;
    // 函数指针类型
    using function_point = R(*)(Args...);
    // 返回值类型
    using return_type = R;
    // 第几个参数的类型
    template<size_t Index>
    using arg = typename args<Index>::type;
};
// 普通函数指针特化
template<typename R, typename ...Args>
struct function_traits<R(*)(Args...)> : function_traits<R(Args...)>{};
// 普通函数引用特化
template<typename R, typename ...Args>
struct function_traits<R(&)(Args...)> : function_traits<R(Args...)>{};

// function特化
template<typename R, typename ...Args>
struct function_traits<function<R(Args...)>> : function_traits<R(Args...)>{};
// function指针特化
template<typename R, typename ...Args>
struct function_traits<function<R(Args...)>*> : function_traits<R(Args...)>{};
// function引用特化
template<typename R, typename ...Args>
struct function_traits<function<R(Args...)>&> : function_traits<R(Args...)>{};

// 类成员函数特化
template<typename R, typename Class, typename ...Args>
struct function_traits<R(Class::*)(Args...)> : function_traits<R(Args...)>{};
// 类const成员函数特化
template<typename R, typename Class, typename ...Args>
struct function_traits<R(Class::*)(Args...) const> : function_traits<R(Args...)>{};
// 类volatile成员函数特化
template<typename R, typename Class, typename ...Args>
struct function_traits<R(Class::*)(Args...) volatile> : function_traits<R(Args...)>{};
// 类const volatile成员函数特化
template<typename R, typename Class, typename ...Args>
struct function_traits<R(Class::*)(Args...) const volatile> : function_traits<R(Args...)>{};

// 可调用对象
template<typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())>{};

#endif //C__11_TEST_FUNCTION_TRAITS_H
