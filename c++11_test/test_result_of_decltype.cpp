//
// Created by root on 7/15/21.
//

#include "iostream"
#include "type_traits"

//result_of在Boost中引入，然后在TR1中引入，最后在C ++ 0x中引入。因此，result_of具有向后兼容(具有合适的库)的优点。
//decltype是C ++ 0x中的全新事物，不仅限于函数的返回类型，而且是语言功能。
//无论如何，在gcc 4.5上，result_of是根据decltype实现的：
//  template<typename _Signature>
//    class result_of;
//
//  template<typename _Functor, typename... _ArgTypes>
//    struct result_of<_Functor(_ArgTypes...)>
//    {
//      typedef
//        decltype( std::declval<_Functor>()(std::declval<_ArgTypes>()...) )
//        type;
//    };

using namespace std;

template <typename F, typename ...Arg>
typename std::result_of<F(Arg...)>::type invoke(F f, Arg ...a)
{
    cout << 3 <<endl;
    return f(forward<Arg>(a)...);
}

//template <typename F, typename Arg>
//auto
//invoke(F f, Arg a) -> decltype(f(a))
//{
//    cout << 1 <<endl;
//    return f(a);
//}

//template <typename F, typename Arg>
//auto invoke(F f, Arg a) -> typename std::result_of<F(Arg)>::type
//{
//    cout << 2 <<endl;
//    return f(a);
//}

int main(){
    invoke([](int a){return 1;}, 1);
    return 0;
}
