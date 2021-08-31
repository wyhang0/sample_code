//
// Created by root on 7/23/21.
//

#include <string>
#include <iostream>
#include <future>

#include "./src/is_detected.h"

using namespace std;

class Box {
public:
    string str(int) {
        return "yes";
    }
    int a;
    using II = int;
};

class Bin {
public:
    string str1() {
        return "no";
    }

    int b;
};

template <typename U>
struct class_str {

    template <typename T, string(T::*)(int) = &T::str>
    static constexpr bool check(T*) { return true; };

    static constexpr bool check(...) { return false; };

    static constexpr bool ret = check(static_cast<U*>(0));
};

//template <typename U>
//struct class_str {
//    template <typename T, string(T::*)() = &T::str>
//    static char check(T*);
//
//    static int check(...);
//
//    const static bool ret = sizeof(check(static_cast<U*>(0))) == sizeof(char);
//};

// 不含有string str()方法的非std::string类
template<typename T,
        typename std::enable_if<std::is_class<T>::value && !std::is_same<T, string>::value, T>::type* = nullptr,
        typename std::enable_if<!class_str<T>::ret, T>::type* = nullptr>
std::string str(T& t) {
    cout << "1.---------------------" << endl;
    return "null";
};

// std::string类
template<typename T,
        typename std::enable_if<std::is_class<T>::value && std::is_same<T, string>::value, T>::type* = nullptr>
std::string str(T& t) {
    cout << "2.---------------------" << endl;
    return t;
};

// 含有string str()方法的非std::string类
template<typename T,
        typename std::enable_if<std::is_class<T>::value && !std::is_same<T, string>::value, T>::type* = nullptr,
        typename std::enable_if<class_str<T>::ret, T>::type* = nullptr>
std::string str(T& t) {
    cout << "3.---------------------" << endl;
    return t.str(1);
};

// 数值型
template<typename T,
        typename std::enable_if<!std::is_class<T>::value && std::is_arithmetic<T>::value, T>::type* = nullptr>
std::string str(T&  t) {
    cout << "4.---------------------" << endl;
    return std::to_string(t);
};

// 是否含有成员函数
template <typename T>
using has_str_t = decltype(declval<T>().str(declval<int>()));
template <typename T>
using has_str = is_detected<has_str_t, T>;
// 是否含有成员变量
template <typename T>
using has_a_t = decltype(declval<T>().a);
template <typename T>
using has_a = is_detected<has_a_t, T>;
// 是否含有内部类型
template <typename T>
using has_II_t = typename T::II;
template <typename T>
using has_II = is_detected<has_II_t, T>;

template<typename T>
using has_get_future_t = decltype(declval<T>().get_future());
template<typename T>
using has_get_future = is_detected<has_get_future_t, T>;

void print(){}
template<typename Head, typename ...Args>
void print(Head head, Args &&...args){
    cout << head << endl;
    print(forward<Args>(args)...);
}
template<typename ...T>
void print(T&&...t){
    print(forward<T>(t)...);
}

int main() {
    string s = "sddds";
    cout << str<string>(s) << endl;

    bool j = true;
    cout << str<bool>(j) << endl;

    int i = 1000;
    cout << str<int>(i) << endl;

    float f = 10.6f;
    cout << str<float>(f) << endl;

    Box b1;
    cout << str<Box>(b1) << endl;

    Bin b2;
    cout << str<Bin>(b2) << endl;

    cout << has_str<Box>::value << endl;
    cout << has_str<Bin>::value << endl;

    cout << has_a<Box>::value << endl;
    cout << has_a<Bin>::value << endl;

    cout << has_II<Box>::value << endl;
    cout << has_II<Bin>::value << endl;

    packaged_task<void()> packagedTask{[]{}};
    cout << has_get_future<decltype(packagedTask)>::value << endl;

    return 0;
}

