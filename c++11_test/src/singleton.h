//
// Created by root on 8/3/21.
//

#ifndef C__11_TEST_SINGLETON_H
#define C__11_TEST_SINGLETON_H

#include <memory>
#include <mutex>
#include <iostream>

using namespace std;

template<typename T>
class Singleton final {
public:
    template<typename ...Args>
    static T* instance(Args&&...args){
        if(m_instance == nullptr){
            std::lock_guard<std::mutex> locker{m_mutex};
            if(m_instance == nullptr){
                unique_ptr<T> tmp(new T{forward<Args>(args)...});
                m_instance = std::move(tmp);
            }
        }
        return m_instance.get();
    }

private:
    Singleton()=default;
    virtual ~Singleton()=default;
    Singleton(const Singleton &)=default;
    Singleton& operator=(const Singleton &)=default;

private:
    static std::unique_ptr<T> m_instance;
    static std::mutex m_mutex;
};

template<typename T>
std::unique_ptr<T> Singleton<T>::m_instance;
template<typename T>
std::mutex Singleton<T>::m_mutex;

#endif //C__11_TEST_SINGLETON_H
