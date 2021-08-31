//
// Created by root on 7/27/21.
//

#ifndef C__11_TEST_TEMPLATE_T_H
#define C__11_TEST_TEMPLATE_T_H

#include <iostream>
#include <tuple>

using namespace std;

// 1.可变参数函数模板
// 1.1递归函数方式展开参数包
void print1(){}
template<typename First, typename ...Rest>
void print1(First &&first, Rest &&...rest){
    cout << first << endl;
    print1(forward<Rest>(rest)...);
}
template<typename ...Args>
void print1(Args&&...args){
    print1(forward<Args>(args)...);
}

// 1.2逗号表达式和初始化列表方式展开参数包
template<typename T>
void print(T &&t){
    cout << t << endl;
}
template<typename ...Args>
void print2(Args&&...args){
#if __cplusplus > 201103L
    initializer_list<int>{([](auto &&arg){cout<<arg<<endl;}(forward<Args>(args)), 0)...};
#else
    initializer_list<int>{(print(forward<Args>(args)), 0)...};
#endif
}

// 2.可变参数类模板
// 2.1模板递归和特化方式展开参数包
#if 0
template<typename ...Args>
struct SumSizeof;
template<typename First, typename ...Rest>
struct SumSizeof<First, Rest...>{
    enum {m_value = SumSizeof<First>::m_value + SumSizeof<Rest...>::m_value};
};
template<typename Last>
struct SumSizeof<Last>{
    enum {m_value = sizeof(Last)};
};
#else
template<typename First, typename ...Rest>
struct SumSizeof{
    enum {value = SumSizeof<First>::value + SumSizeof<Rest...>::value};
};
template<typename Last>
struct SumSizeof<Last>{
    enum {value = sizeof(Last)};
};
#endif
// 2.2通过继承与特化展开参数包
#if __cplusplus > 201103L
// 自定义tuple
template<int N, typename ...Args>
struct _Members{};
template<int N, typename First, typename ...Rest>
struct _Members<N,First,Rest...> : _Members<N+1, Rest...>{
    _Members(){}
    _Members(First &&first, Rest &&...rest)
            :value(forward<First>(first)), _Members<N + 1, Rest...>(forward<Rest>(rest)...){
    }
    void print(){
        cout << value << endl;
        _Members<N+1, Rest...>::print();
    }
    template<size_t Index, typename T, typename enable_if<Index==N, T>::type* = nullptr>
    void set(T &&t){
        value = forward<T>(t);
    }
    template<size_t Index, typename T, typename enable_if<Index!=N, T>::type* = nullptr>
    void set(T &&t){
       _Members<N+1, Rest...>::template set<Index>(forward<T>(t));
    }
    template<size_t Index, typename enable_if<Index==N>::type* = nullptr>
    First& get(){
        return value;
    }
    template<size_t Index, typename enable_if<Index!=N>::type* = nullptr>
    auto& get(){
        return _Members<N+1, Rest...>::template get<Index>();
    }

    First value;
};
template<int N>
struct _Members<N>{
    void print(){}
};
template<typename ...Args>
struct Members : _Members<0, Args...>{
    Members(){}
    Members(Args &&...args):_Members<0, Args...>(forward<Args>(args)...){}
    void print(){
        _Members<0, Args...>::print();
    }
    template<size_t Index, typename T, typename enable_if<Index<sizeof...(Args), T>::type* = nullptr>
    void set(T &&t){
        _Members<0, Args...>::template set<Index>(forward<T>(t));
    }
    template<size_t Index, typename enable_if<Index<sizeof...(Args)>::type* = nullptr>
    auto& get(){
        return _Members<0, Args...>::template get<Index>();
    }

    size_t size = sizeof...(Args);
};
template<>
struct Members<>{
    void print(){}
    size_t size = 0;
};
#endif

// 3.打印tuple
template<int...>
struct IndexSeq{};

template<int N, int...Indexes>
struct MakeIndexes{
    using type = typename MakeIndexes<N - 1, N - 1, Indexes...>::type;
};

template<int...Indexes>
struct MakeIndexes<0, Indexes...>{
    using type = IndexSeq<Indexes...>;
};

// 打印tuple
template<typename Tuple>
void print_tuple(Tuple &&tup){
    using Index = typename MakeIndexes<tuple_size<typename decay<Tuple>::type>::value >::type;
    __print_tuple(Index{}, forward<decltype(tup)>(tup));
}
template<int ...Indexes, typename Tuple>
void __print_tuple(IndexSeq<Indexes...> &&, Tuple &&tup){
#if __cplusplus > 201103L
    initializer_list<int>{([](auto &&i){cout<<i<<endl;}(get<Indexes>(tup)),0)...};
#else
    initializer_list<int>{(print(get<Indexes>(tup)),0)...};
#endif
}

template<size_t N, typename ...Args, typename enable_if<N==sizeof...(Args)>::type* = nullptr>
void print_tuple(size_t index, tuple<Args...> &tup){
    throw out_of_range("index out of range");
}
template<size_t N=0, typename ...Args, typename enable_if<N<sizeof...(Args)>::type* = nullptr>
void print_tuple(size_t index, tuple<Args...> &tup){
    if(index == N)
        cout << get<N>(tup) << endl;
    else
        print_tuple<N+1>(index, tup);
}

// 判断类型T是否在List列表中, eg: ContainerType<double>::value
template<typename T, typename ...List>
struct ContainerType;
template<typename T, typename First, typename ...Rest>
struct ContainerType<T, First, Rest...> : conditional<is_same<T, First>::value, true_type, ContainerType<T, Rest...>>::type{};
template<typename T>
struct ContainerType<T> : false_type {};

// 获取类型T在List列表中的序号, eg: TypeIndex<double>::value
template<int Index, typename T, typename ...List>
struct _TypeIndex;
template<int Index, typename T, typename First, typename ...Rest>
struct _TypeIndex<Index, T, First, Rest...> : integral_constant<int, is_same<T, First>::value ? Index : _TypeIndex<Index+1, T, Rest...>::value>{};
template<int Index, typename T>
struct _TypeIndex<Index, T> : integral_constant<int, -1> {};
template<typename T, typename ...List>
struct TypeIndex : _TypeIndex<0, T, List...>{};

// 获取List中序号为Index的类型, eg: IndexType<0, double>::type
template<size_t Index, typename ...List>
struct IndexType{
    static_assert(Index < sizeof...(List), "index out of range");
};
template<size_t Index, typename First, typename ...Rest>
struct IndexType<Index, First, Rest...>{
    using type = typename conditional<Index==0, First, typename IndexType<Index-1, Rest...>::type>::type;
};
template<typename First, typename ...Rest>
struct IndexType<0, First, Rest...>{
    using type = First;
};


#endif //C__11_TEST_TEMPLATE_T_H
