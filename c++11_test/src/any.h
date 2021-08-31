//
// Created by root on 7/29/21.
//

#ifndef C__11_TEST_ANY_H
#define C__11_TEST_ANY_H

#include <memory>
#include <typeindex>
#include <iostream>

#include "./function_traits.h"

using namespace std;

class Any{
public:
    Any(): m_tyIndex(typeid(void)){}
    Any(const Any &o) : m_tyIndex(o.m_tyIndex), m_ptr(o.clone()){}
    Any(Any &&o) : m_tyIndex(o.m_tyIndex), m_ptr(move(o.m_ptr)){}
    template<typename T, typename enable_if<!is_same<typename decay<T>::type, Any>::value>::type* = nullptr>
    Any(T &&t) : m_tyIndex(typeid(typename decay<T>::type)){
        using Type = typename decay<T>::type;
        BasePtr tmp{new Derived<Type>{forward<T>(t)}};
        m_ptr = move(tmp);
    }

    bool isNull() const {
        return m_ptr == nullptr;
    }

    template<typename T>
    bool is() const {
        return m_tyIndex == type_index(typeid(T));
    }

    void visit(){}
    template<typename F, typename ...Rest>
    void visit(F &&f, Rest&&...rest){
        using T = typename decay<typename function_traits<F>::template arg<0>>::type;
        if(is<T>())
            f(any_cast<T>());
        else
            visit(forward<Rest>(rest)...);
    }

    template<typename T>
    T& any_cast() const noexcept(false) {
        if(!is<T>()){
            cout << "can not cast " << typeid(T).name() << " to " << m_tyIndex.name() << endl;
            throw bad_cast();
        }
        auto derived = dynamic_cast<Derived<T>*>(m_ptr.get());
        return derived->m_value;
    }
    void* point_cast() {
        return m_ptr.get()->point();
    }

    Any& operator=(const Any &o){
        if(&o == this)
            return *this;
        m_tyIndex = o.m_tyIndex;
        m_ptr = o.clone();
        return *this;
    }

    Any& operator=(Any &&o){
        m_tyIndex = o.m_tyIndex;
        m_ptr = move(o.m_ptr);
        return *this;
    }


private:
    struct Base;
    using BasePtr = unique_ptr<Base>;

    struct Base{
        virtual ~Base(){}
        virtual BasePtr clone() const = 0;
        virtual void* point() const=0;
    };

    template<typename T>
    struct Derived : Base{
        template<typename U>
        Derived(U &&value): m_value(forward<U>(value)){}
        template<typename U>
        Derived(U &value): m_value(forward<U>(value)){}
        BasePtr clone() const{
            return BasePtr(new Derived<T>(m_value));
        }
        void* point() const{
            return (void*)&m_value;
        }

        T m_value;
    };

    BasePtr clone() const{
        if(m_ptr != nullptr)
            return m_ptr->clone();
        return nullptr;
    }

    BasePtr m_ptr;
    type_index m_tyIndex;
};

#endif //C__11_TEST_ANY_H
