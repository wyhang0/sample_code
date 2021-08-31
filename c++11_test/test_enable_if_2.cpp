//
// Created by root on 7/14/21.
//

#include <iostream>
#include <type_traits>
#include <functional>

//用法二：控制函数返回类型
//对于模板函数，有时希望根据不同的模板参数返回不同类型的值，进而给函数模板也赋予类型模板特化的性质。典型的例子可以参看 tuple 的获取第 k 个元素的 get 函数：

using namespace std;

template<typename T>
struct A{
#if 0
    template<typename TT=T>
    typename enable_if<is_pointer<TT>::m_value, int>::type m_value(){
        return 1;
    }

    template<typename TT=T>
    typename enable_if<is_same<TT, bool>::m_value, int>::type m_value(){
        return 2;
    }

    template<typename TT=T>
    typename enable_if<is_same<TT, int>::m_value, int>::type m_value(){
        return 3;
    }

    template<typename TT=T>
    typename enable_if<is_same<TT, reference_wrapper<int>>::m_value, int>::type m_value(){
        return 4;
    }
#else
    template<typename TT=T, typename enable_if<is_pointer<TT>::value, TT>::type* = nullptr>
    int value(){
        return 1;
    }

    template<typename TT=T, typename enable_if<is_same<TT, bool>::value, TT>::type* = nullptr>
    int value(){
        return 2;
    }

    template<typename TT=T, typename enable_if<is_same<TT, int>::value, TT>::type* = nullptr>
    int value(){
        return 3;
    }

    template<typename TT=T, typename enable_if<is_same<TT, reference_wrapper<int>>::value, TT>::type* = nullptr>
    int value(){
        return 4;
    }
#endif
};

int main(){
    cout << A<void*>().value() << endl;
    cout << A<bool>().value() << endl;
    cout << A<int>().value() << endl;
    cout << A<reference_wrapper<int>>().value() << endl;
    return 0;
}
