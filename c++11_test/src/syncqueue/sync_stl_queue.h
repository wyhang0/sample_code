//
// Created by ubuntu on 2021/7/16.
//

#ifndef C__11_TEST_SYNC_STL_QUEUE_H
#define C__11_TEST_SYNC_STL_QUEUE_H

#include "sync_queue.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <list>

using namespace std;
using namespace chrono;

/**
 * 读写线程同步执行。
 * @tparam T
 */
template<typename T>
class SyncStlQueue final : public SyncQueue<T>{
public:
    explicit SyncStlQueue(size_t size=256);

    SyncStlQueue(const SyncStlQueue<T> &)=delete;

    bool push(T &&t, microseconds duration=microseconds{250});

    template<typename ...Args>
    bool emplace(microseconds duration, Args&&...args);

    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value, bool>::type pop(T &t, microseconds duration=microseconds{250});

    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value, bool>::type pop(T *t, microseconds duration=microseconds{250});

    size_t size();

private:
    size_t m_size{};
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_notEmpty;
    condition_variable m_notFull;
};

template<typename T>
SyncStlQueue<T>::SyncStlQueue(size_t size) :m_size(size) {

}

template<typename T>
bool SyncStlQueue<T>::push(T &&t, microseconds duration) {
    {
        unique_lock<mutex> locker{m_mutex};
        if(!m_notFull.wait_for(locker, duration, [this]{return m_queue.size() < m_size;})){
            return false;
        }
        m_queue.push(forward<T>(t));
    }
    m_notEmpty.notify_one();
    return true;
}

template<typename T>
template<typename... Args>
bool SyncStlQueue<T>::emplace(microseconds duration, Args&&...args) {
    {
        unique_lock<mutex> locker{m_mutex};
        if(!m_notFull.wait_for(locker, duration, [this]{return m_queue.size() < m_size;})){
            return false;
        }
        m_queue.emplace(forward<Args>(args)...);
    }
    m_notEmpty.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value, bool>::type SyncStlQueue<T>::pop(T &t, microseconds duration) {
    {
        unique_lock<mutex> locker{m_mutex};
        if(!m_notEmpty.template wait_for(locker, duration, [this]{return m_queue.size()>0;})){
            return false;
        }
        t = m_queue.front();
        m_queue.pop();
    }
    m_notFull.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value, bool>::type SyncStlQueue<T>::pop(T *t, microseconds duration) {
    {
        unique_lock<mutex> locker{m_mutex};
        if(!m_notEmpty.template wait_for(locker, duration, [this]{return m_queue.size()>0;})){
            return false;
        }
        *t = m_queue.front();
        m_queue.pop();
    }
    m_notFull.notify_one();
    return true;
}

template<typename T>
size_t SyncStlQueue<T>::size() {
    lock_guard<mutex> lockGuard{m_mutex};
    return m_queue.size();
}

#endif //C__11_TEST_SYNC_STL_QUEUE_H
