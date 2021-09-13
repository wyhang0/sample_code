//
// Created by ubuntu on 2021/8/21.
//

#ifndef C__11_TEST_VARIANT_H
#define C__11_TEST_VARIANT_H

#include <type_traits>
#include <typeindex>
#include <iostream>

#include "./function_traits.h"
#include "./template....T.h"

using namespace std;

template<typename First, typename ...Rest>
struct MaxType : integral_constant<size_t ,(sizeof(First)>MaxType<Rest...>::value ? sizeof(First) : MaxType<Rest...>::value)>{};
template<typename Last>
struct MaxType<Last> : integral_constant<size_t, sizeof(Last)>{};

template<typename First, typename ...Rest>
struct MaxAlign : integral_constant<size_t ,(alignof(First)>MaxAlign<Rest...>::value ? alignof(First) : MaxAlign<Rest...>::value)>{};
template<typename Last>
struct MaxAlign<Last> : integral_constant<size_t, alignof(Last)>{};

template<typename... Args>
struct VariantHelper;
template<typename T, typename... Args>
struct VariantHelper<T, Args...> {
    inline static void destroy(type_index id, void *data)
    {
        if (id == type_index(typeid(T)))
            ((T*) (data))->~T();
        else
            VariantHelper<Args...>::destroy(id, data);
    }
    inline static void copy(type_index id, void *src, void *dest)
    {
        if (id == type_index(typeid(T)))
            new (dest) T(*reinterpret_cast<T*>(src));
        else
            VariantHelper<Args...>::copy(id, src, dest);
    }
    inline static void move(type_index id, void *src, void *dest)
    {
        if (id == type_index(typeid(T)))
            new (dest) T(std::move(*reinterpret_cast<T*>(src)));
        else
            VariantHelper<Args...>::move(id, src, dest);
    }
};
template<>
struct VariantHelper<>  {
    inline static void destroy(type_index id, void *data) {}
    inline static void copy(type_index id, void *src, void *dest){}
    inline static void move(type_index id, void *src, void *dest){}
};

template<typename ...Types>
class Variant{
public:
    enum {
        data_size = MaxType<Types...>::value,
        align_size = MaxAlign<Types...>::value
    };
    using Helper_t = VariantHelper<Types...>;

    Variant(void) : m_typeIndex(typeid(void)){

    }

    ~Variant(){
        Helper_t::destroy(m_typeIndex, &m_data);
    }

    Variant(Variant<Types...>&& old){
        Helper_t::destroy(m_typeIndex, &m_data);
        m_typeIndex = old.m_typeIndex;
        Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
    }

    Variant(const Variant<Types...>& old){
        Helper_t::destroy(m_typeIndex, &m_data);
        m_typeIndex = old.m_typeIndex;
        Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
    }

    Variant& operator=(const Variant& old)
    {
        Helper_t::destroy(m_typeIndex, &m_data);
        Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
        m_typeIndex = old.m_typeIndex;
        return *this;
    }

    Variant& operator=(Variant&& old)
    {
        Helper_t::destroy(m_typeIndex, &m_data);
        Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
        m_typeIndex = old.m_typeIndex;
        return *this;
    }

    template<typename T, typename = typename enable_if<ContainerType<typename remove_reference<T>::type, Types...>::value>::type>
    Variant(T &&t){
        Helper_t::destroy(m_typeIndex, (void*)&m_data);
        using U = typename std::remove_reference<T>::type;
        new (&m_data) U(std::forward<T>(t));
        m_typeIndex = type_index(typeid(U));
    }

    bool empty() const{
        return m_typeIndex == type_index(typeid(void));
    }

    template<typename T>
    bool is() const{
        return (m_typeIndex == type_index(typeid(T)));
    }

    type_index type() const{
        return m_typeIndex;
    }

    template<typename T>
    T& get()
    {
        if(!is<T>()){
            cout << typeid(T).name() << " is not defined. " << "current type is " << m_typeIndex.name() << endl;
            throw bad_cast();
        }
        return *(T*)(&m_data);
    }

    template<typename T>
    int getIndexOf() const{
        return TypeIndex<T, Types...>::value;
    }

    template<size_t N>
    auto getIndexType() -> typename IndexType<N, Types...>::type;

    template<typename F>
    void visit(F&& f)
    {
        using T = typename remove_reference<typename function_traits<F>::template arg<0>::type>::type;
        if (is<T>())
            f(get<T>());
    }

    template<typename F, typename... Rest>
    void visit(F&& f, Rest&&... rest)
    {
        using T = typename remove_reference<typename function_traits<F>::template arg<0>::type>::type;
        if (is<T>())
            visit(std::forward<F>(f));
        else
            visit(std::forward<Rest>(rest)...);
    }

private:
    typename aligned_storage<data_size, align_size>::type m_data;
    type_index m_typeIndex{typeid(void)};
};

#endif //C__11_TEST_VARIANT_H
