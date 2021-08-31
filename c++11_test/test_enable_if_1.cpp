//
// Created by root on 7/14/21.
//

// 用法一：类型偏特化
//在使用模板编程时，经常会用到根据模板参数的某些特性进行不同类型的选择，或者在编译时校验模板参数的某些特性。

#include <iostream>
#include <type_traits>

using namespace std;

template<typename T, typename Enable = void>
struct check;

template<typename T>
struct check<T, typename enable_if<is_pointer<T>::value>::type>
{
    using type = T;
    type a;
};

struct A{
    static const bool value = true;
};

int main(){
    check<A*> a;
    cout << a.a->value << endl;

    return 0;
}

