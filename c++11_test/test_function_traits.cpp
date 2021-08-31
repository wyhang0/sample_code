//
// Created by root on 7/30/21.
//

#include <iostream>
#include <typeinfo>
#include <functional>

#include "./src/function_traits.h"

using namespace std;

int add(int i, int j){
    return i+j;
}

struct AA{
    double f(int i){
        return i;
    }
    double f(int i) const{
        return i;
    }
    double f(int i) volatile{
        return i;
    }
    double f(int i) const volatile{
        return i;
    }
    float operator()(int i) const{
        return i;
    }
};
struct BB{
    int operator()(int i) const{
        return i;
    }
    char operator()(char i){
        return i;
    }
};

int main(){
    // 普通函数
    {
        using Fn = int(int, int);
        using FnP = int (*)(int, int);
        using FnR = int (&)(int, int);
        Fn *f1 = add;
        Fn &f2 = add;
        Fn *f3 = [](int i, int j) { return i + j; }; // lambda表达式不捕获参数可以转换为普通函数指针
        auto f4 = [](int i, int j) { return i + j; };
        Fn *f5 = f4;
        cout << typeid(function_traits<typename remove_cv<decltype(f5)>::type>::return_type).name() << endl;
        cout << typeid(function_traits<typename remove_cv<decltype(f1)>::type>::return_type).name() << endl;
        cout << typeid(function_traits<typename remove_cv<decltype(f2)>::type>::return_type).name() << endl;
    }
    // function
    {
        function<int(int)> f1;
        function<int(int)> *f2;
        function<int(int)> &f3 = f1;
        cout << typeid(function_traits<decltype(f1)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(f2)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(f3)>::return_type).name() << endl;
    }
    // class
    {
        using C1 = double(AA::*)(int);
        using C2 = double(AA::*)(int) const;
        using C3 = double(AA::*)(int) volatile;
        using C4 = double(AA::*)(int) const volatile;
        C1 c1 = &AA::f;
        C2 c2 = &AA::f;
        C3 c3 = &AA::f;
        C4 c4 = &AA::f;
        cout << typeid(function_traits<decltype(c1)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(c2)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(c3)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(c4)>::return_type).name() << endl;
    }
    // 可调用对象1
    {
        using C1 = decltype(&AA::operator());
        using C2 = float(AA::*)(int) const;
        C1 c1 = &AA::operator();
        C2 c2 = &AA::operator();
        cout << typeid(function_traits<decltype(c1)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(c2)>::return_type).name() << endl;
        cout << typeid(function_traits<AA>::return_type).name() << endl;
    }
    // 可调用对象2
    {
        using C1 = int(BB::*)(int) const;
        using C2 = char(BB::*)(char );
        C1 c1 = &BB::operator();
        C2 c2 = &BB::operator();
        cout << typeid(function_traits<decltype(c1)>::return_type).name() << endl;
        cout << typeid(function_traits<decltype(c2)>::return_type).name() << endl;
    }
    return 0;
}
