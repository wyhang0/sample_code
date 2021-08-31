//
// Created by root on 8/5/21.
//

#ifndef C__11_TEST_CLASS_OBJECT_POOL_H
#define C__11_TEST_CLASS_OBJECT_POOL_H

#include <memory>
#include <functional>
#include <map>
#include <string>
#include <typeindex>

#include "non_copyable.h"

using namespace std;

#if 0
template<typename T>
class ClassObjectPool : NonCopyable{
public:
    using DeleteType = function<void(T *)>;
    template<typename ...Args>
    using Constructor = void(T::*)(typename decay<Args>::type...);

    /**
     * 根据void(T::*)(decay_t<Args>...)的类型区分构造函数.
     * Args中的所有类型去除引用.数组类型变为去除顶层维度的指针,函数类型变为函数类型指针,其他类型去除const volatile.
     * eg: "segg"->const char &[5]->const char *; int(&)(int,int)->int(*)(int,int); const int& -> int
     * @tparam Args
     * @param num
     * 类型Ｔ的个数
     * @param args
     * 类型T构造函数的参数
     */
    template<typename ...Args>
    void init(size_t num, Args&&...args){
        auto constructorName = typeid(Constructor<Args...>).name();
        for (int i = 0; i < num; ++i) {
            m_map.template emplace(constructorName, unique_ptr<T>{new T{forward<Args>(args)...}});
        }
    }
    template<typename ...Args>
    size_t clear(){
        auto constructorName = typeid(Constructor<Args...>).name();
        return m_map.erase(constructorName);
    }

    template<typename ...Args>
    unique_ptr<T, DeleteType> get(){
        auto constructorName = typeid(Constructor<Args...>).name();
        auto item = m_map.find(constructorName);
        if(item == m_map.end())
            return nullptr;
        auto tmp = move(item->second);
        m_map.erase(item);

        // -----------------
        unique_ptr<T, DeleteType> ptr(tmp.release(), [this, constructorName](T *t){
            m_map.template emplace(constructorName, unique_ptr<T>{t});
        });
        return move(ptr);
    }

private:
    multimap<string, unique_ptr<T>> m_map;
};
#else
/**
 * 对象有移动构造函数,并且执行移动构造代价较小时可以使用.
 */
template<typename T>
class ClassObjectPool : NonCopyable{
public:
    template<typename ...Args>
    using Constructor = void(T::*)(typename decay<Args>::type...);

    /**
     * 根据void(T::*)(decay_t<Args>...)的类型区分构造函数.
     * Args中的所有类型去除引用.数组类型变为去除顶层维度的指针,函数类型变为函数类型指针,其他类型去除const volatile.
     * eg: "segg"->const char &[5]->const char *; int(&)(int,int)->int(*)(int,int); const int& -> int
     * @tparam Args
     * @param num
     * 类型Ｔ的个数
     * @param args
     * 类型T构造函数的参数
     */
    template<typename ...Args>
    void init(size_t num, Args&&...args){
        auto constructorName = typeid(Constructor<Args...>).name();
        for (int i = 0; i < num; ++i) {
            m_map.template emplace(constructorName, shared_ptr<T>{new T{forward<Args>(args)...}});
        }
    }

    template<typename ...Args>
    shared_ptr<T> get(){
        auto constructorName = typeid(Constructor<Args...>).name();
        auto item = m_map.find(constructorName);
        if(item == m_map.end())
            return nullptr;
        auto tmp = item->second;
        m_map.erase(item);

        // -----------------
        shared_ptr<T> ptr(new T(*tmp), [this, constructorName](T *t){
            m_map.template emplace(constructorName, shared_ptr<T>{t});
        });
        return ptr;
    }

private:
    multimap<string, shared_ptr<T>> m_map;
};
#endif

#endif //C__11_TEST_CLASS_OBJECT_POOL_H
