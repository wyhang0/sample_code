//
// Created by root on 8/10/21.
//

#ifndef C__11_TEST_IOC_CONTAINER_H
#define C__11_TEST_IOC_CONTAINER_H

#include <functional>
#include <memory>
#include <map>

#include "./non_copyable.h"
#include "./any.h"

using namespace std;

class IocContainer : NonCopyable{
public:
    // 创建继承关系的对象
    template<typename Base, typename Derived, typename ...DerivedArgs, typename enable_if<is_base_of<Base, Derived>::value>::type* = nullptr>
    void registerType(const string &key){
        function<Base*(DerivedArgs...)> f = [](DerivedArgs&&...derivedArgs){
            return new Derived{forward<DerivedArgs>(derivedArgs)...};
        };
        registerType(key, f);
    }
    // 创建依赖关系的对象,TArgs为Ｔ构造函数除了Depend×外的参数. eg: registerType<T, Depend, Targs...>("ar2")
    template<typename T, typename Depend, typename ...TArgs, typename enable_if<!is_base_of<T, Depend>::value>::type* = nullptr>
    void registerType(const string &key){
        function<T*(TArgs...)> f = [](TArgs&&...tArgs){
            return new T{forward<TArgs>(tArgs)..., new Depend{}};
        };
        registerType(key, f);
    }
    //　链式构建依赖关系
    // 创建依赖关系的对象,TArgs为Ｔ构造函数除了Depend×外的参数,　DependArgs为Depend构造函数的参数, dependKey为Depend构造函数的key.　eg: registerType<T, Depend, Targs...>("ar2", "car2", DependArgs...)
    template<typename T, typename Depend, typename ...TArgs, typename ...DependArgs, typename enable_if<!is_base_of<T, Depend>::value>::type* = nullptr>
    void registerType(const string &key, const string &dependKey, DependArgs&&...dependArgs){
        function<T*(TArgs..., DependArgs...)> f = [this, dependKey](TArgs&&...tArgs, DependArgs&&...dependArgs){
            auto item = m_creatorMap.find(dependKey);
            if(item == m_creatorMap.end())
                throw invalid_argument("this key isn't exist");
            Any any = item->second;
            auto dependConstructor = any.any_cast<function<Depend*(DependArgs...)>>();
            return new T{forward<TArgs>(tArgs)..., dependConstructor(forward<DependArgs>(dependArgs)...)};
        };
        registerType(key, f);
    }
    // 创建普通对象
    template<typename T, typename ...Args>
    void registerSimple(const string &key){
        function<T*(Args...)> f = [](Args&&...args){
            return new T{forward<Args>(args)...};
        };
        registerType(key, f);
    }
    // 获取对象
    template<typename T, typename ...TArgs, typename ...DependArgs>
    shared_ptr<T> resolveShared(const string &key, TArgs&&...targs, DependArgs&&...dependArgs){
        auto item = m_creatorMap.find(key);
        if(item == m_creatorMap.end())
            return nullptr;
        Any resolver = item->second;
        auto f = resolver.any_cast<function<T*(TArgs..., DependArgs...)>>();
        T *t = f(forward<TArgs>(targs)..., forward<DependArgs>(dependArgs)...);
        return shared_ptr<T>(t);
    }
private:
    void registerType(const string &key, Any constructor){
        if(m_creatorMap.find(key) != m_creatorMap.end())
            throw invalid_argument("this key has already exist");
        m_creatorMap.emplace(key, constructor);
    }

private:
    map<string, Any> m_creatorMap;
};

#endif //C__11_TEST_IOC_CONTAINER_H
