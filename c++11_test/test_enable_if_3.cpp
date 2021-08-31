//
// Created by root on 7/15/21.
//

#include <iostream>
#include <type_traits>
#include <functional>

//用法三：校验函数模板参数类型
//有时定义的模板函数，只希望特定的类型可以调用，参考 cppreference 官网示例，很好的说明了如何限制只有整型可以调用的函数定义：

using namespace std;

struct A{
#if 0
    template<typename T>
    typename enable_if<is_pointer<T>::m_value, int>::type m_value(T t){
        return 1;
    }
    template<typename T>
    typename enable_if<is_same<T, bool>::m_value, int>::type m_value(T t){
        return 2;
    }
    template<typename T>
    typename enable_if<is_same<T, int>::m_value, int>::type m_value(T t){
        return 3;
    }
    template<typename T>
    typename enable_if<is_same<T, reference_wrapper<int>>::m_value, int>::type m_value(T t){
        return intReference(t);
    }

    int intReference(int &t){
        t = 2;
        return 4;
    }
#else
    template<typename T, typename enable_if<is_pointer<T>::value, T>::type* = nullptr>
     int value(T t){
        return 1;
    }
    template<typename T, typename enable_if<is_same<T, bool>::value, T>::type* = nullptr>
    int value(T t){
        return 2;
    }
    template<typename T, typename enable_if<is_same<T, int>::value, T>::type* = nullptr>
    int value(T t){
        return 3;
    }
    template<typename T, typename enable_if<is_same<T, reference_wrapper<int>>::value, T>::type* = nullptr>
    int value(T t){
        return intReference(t);
    }

    int intReference(int &t){
        t = 2;
        return 4;
    }
#endif
};

int main(){
    struct A a;
    int  b = 0;
    int &c = b;
    cout << a.value((void*)0) << endl;
    cout << a.value(true) << endl;
    cout << a.value(b) << endl;
    cout << a.value(c) << endl;
    cout << a.value(ref(c)) << endl;

    return 0;
}
