//
// Created by ubuntu on 2021/7/17.
//

#ifndef C__11_TEST_SYNC_RING_ARRAY_V1_H
#define C__11_TEST_SYNC_RING_ARRAY_V1_H

#include "sync_queue.h"

#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;
using namespace chrono;

/**
 * v1
 * 读写线程并发执行，读线程同步执行，写线程同步执行。1:1生产者消费者模型最快，每增加一对，性能指数级下降。
 * 1-1消费模型最快,n-n消费模型每增加一对多花费[1-1]*0.7s.
 * @tparam T
 */
#if 1
template<typename T>
class SyncRingArrayV1 : public SyncQueue<T>{
public:
    explicit SyncRingArrayV1(size_t size=256);
    ~SyncRingArrayV1();
    SyncRingArrayV1(const SyncRingArrayV1<T> &)=delete;

    bool push(T &&t, microseconds duration=microseconds{250});
    bool push(const T &t, microseconds duration=microseconds{250});

    template<typename ...Args>
    bool emplace(microseconds duration, Args&&...args);

    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value, bool>::type pop(T &t, microseconds duration=microseconds{250});

    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value, bool>::type pop(T *t, microseconds duration=microseconds{250});

    size_t size();

private:
    template<typename TT>
    bool _push(TT &&t, microseconds duration=microseconds{250});
    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value>::type clearData(void *isPoint=nullptr);
    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value>::type clearData(int isNotPoint=1);
private:
    using Aligned_T = typename aligned_storage<sizeof(T), alignof(T)>::type;
    size_t m_size;
    size_t m_mask;
    Aligned_T *m_queue;

    size_t m_head{0};
    mutex m_notEmptyMt;
    condition_variable m_notEmptyCv;

    size_t m_tail{0};
    mutex m_notFullMt;
    condition_variable m_notFullCv;
};

template<typename T>
SyncRingArrayV1<T>::SyncRingArrayV1(size_t size) {
    if(size < 16)
        size = 16;
    // 保证size是2的幂次
    if((size&(size-1))!=0){
        // 1.得到最高位为1其他位为0时的值
        size_t t1 = size;
        size_t t2 = t1&(t1-1);
        while(t2 !=0){
            t1 = t2;
            t2 = t1&(t1 -1);
        }
        // 2.大于size值的2次幂值
        t1 = t1<<1;
        size = t1;
    }
    m_queue = new Aligned_T[size];
    m_size = size;
    m_mask = m_size - 1;
    m_head = m_tail = 0;
}

template<typename T>
SyncRingArrayV1<T>::~SyncRingArrayV1() {
    if(m_queue){
        clearData();
        delete[] m_queue;
        m_queue = nullptr;
    }
}

template<typename T>
bool SyncRingArrayV1<T>::push(T &&t, microseconds duration) {
    return _push(move(t), duration);
}

template<typename T>
bool SyncRingArrayV1<T>::push(const T &t, microseconds duration) {
    return _push(t, duration);
}

template<typename T>
template<typename ...Args>
bool SyncRingArrayV1<T>::emplace(microseconds duration, Args &&... args) {
    {
        unique_lock<mutex> locker(m_notFullMt);
        if(!m_notFullCv.wait_for(locker, duration, [this]{return m_tail-m_head < m_mask;}))
            return false;
        new(&m_queue[m_tail & m_mask]) T{forward<Args>(args)...};
        m_tail++;
    }
    m_notEmptyCv.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value, bool>::type SyncRingArrayV1<T>::pop(T &t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notEmptyMt);
        if(!m_notEmptyCv.wait_for(locker, duration, [this] {return m_head != m_tail;}))
            return false;
        t = move(*reinterpret_cast<T*>(&m_queue[m_head & m_mask]));
        // 执行析构函数，防止没有移动赋值函数,调用拷贝赋值函数后不释放queue的对象.如果对象有指针成员会造成内存泄露.
        reinterpret_cast<T*>(&m_queue[m_head & m_mask])->~T();
        m_head++;
    }
    m_notFullCv.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value, bool>::type SyncRingArrayV1<T>::pop(T *t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notEmptyMt);
        if(!m_notEmptyCv.wait_for(locker, duration, [this] {return m_head != m_tail;}))
            return false;
        *t = *reinterpret_cast<T*>(&m_queue[m_head & m_mask]);
        m_head++;
    }
    m_notFullCv.notify_one();
    return false;
}

template<typename T>
size_t SyncRingArrayV1<T>::size() {
    size_t len = m_tail - m_head;
    if(len > m_mask)
        len = m_tail - (m_head & m_mask) + m_size;
    return len;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value>::type SyncRingArrayV1<T>::clearData(void *isPoint) {
    while(m_head!=m_tail) {
        // 释放堆内存
        delete *reinterpret_cast<T*>(&m_queue[m_head&m_mask]);
        m_head++;
    }
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value>::type SyncRingArrayV1<T>::clearData(int isNotPoint) {
    while(m_head!=m_tail) {
        // 执行析构函数，防止对象有指针成员会造成内存泄露.
        reinterpret_cast<T*>(&m_queue[m_head&m_mask])->~T();
        m_head++;
    }
}

template<typename T>
template<typename TT>
bool SyncRingArrayV1<T>::_push(TT &&t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notFullMt);
        if(!m_notFullCv.wait_for(locker, duration, [this]{return m_tail-m_head < m_mask;}))
            return false;
        new(&m_queue[m_tail & m_mask]) T{forward<T>(t)};
        m_tail++;
    }
    m_notEmptyCv.notify_one();
    return true;
}

#else

template<typename T>
class SyncRingArrayV1 : public SyncQueue<T>{
public:
    explicit SyncRingArrayV1(size_t size=1024);
    ~SyncRingArrayV1();
    SyncRingArrayV1(const SyncRingArrayV1<T> &)=delete;

    bool push(T &&t, microseconds duration=microseconds{250});
    bool push(const T &t, microseconds duration=microseconds{250});

    template<typename ...Args>
    bool emplace(microseconds duration, Args&&...args);

    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value, bool>::type pop(T &t, microseconds duration=microseconds{250});

    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value, bool>::type pop(T *t, microseconds duration=microseconds{250});

    size_t size();

private:
    template<typename TT>
    bool _push(TT &&t, microseconds duration=microseconds{250});
    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value>::type clearData(void *isPoint=nullptr);
    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value>::type clearData(int isNotPoint=1);
private:
    size_t m_size;
    size_t m_mask;
    T *m_queue;

    size_t m_head{0};
    mutex m_notEmptyMt;
    condition_variable m_notEmptyCv;

    size_t m_tail{0};
    mutex m_notFullMt;
    condition_variable m_notFullCv;
};

template<typename T>
SyncRingArrayV1<T>::SyncRingArrayV1(size_t size) {
    if(size < 16)
        size = 16;
    // 保证size是2的幂次
    if((size&(size-1))!=0){
        // 1.得到最高位为1其他位为0时的值
        size_t t1 = size;
        size_t t2 = t1&(t1-1);
        while(t2 !=0){
            t1 = t2;
            t2 = t1&(t1 -1);
        }
        // 2.大于size值的2次幂值
        t1 = t1<<1;
        size = t1;
    }
    m_queue = new T[size];
    m_size = size;
    m_mask = m_size - 1;
    m_head = m_tail = 0;

}

template<typename T>
SyncRingArrayV1<T>::~SyncRingArrayV1() {
    if(m_queue){
        clearData();
        delete[] m_queue;
        m_queue = nullptr;
    }
}

template<typename T>
bool SyncRingArrayV1<T>::push(T &&t, microseconds duration) {
    return _push(move(t), duration);
}

template<typename T>
bool SyncRingArrayV1<T>::push(const T &t, microseconds duration) {
    return _push(t, duration);
}

template<typename T>
template<typename ...Args>
bool SyncRingArrayV1<T>::emplace(microseconds duration, Args &&... args) {
    {
        unique_lock<mutex> locker(m_notFullMt);
        if(!m_notFullCv.wait_for(locker, duration, [this]{return m_tail-m_head < m_mask;}))
            return false;
        new(&m_queue[m_tail & m_mask]) T{forward<Args>(args)...};
        m_tail++;
    }
    m_notEmptyCv.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value, bool>::type SyncRingArrayV1<T>::pop(T &t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notEmptyMt);
        if(!m_notEmptyCv.wait_for(locker, duration, [this] {return m_head != m_tail;}))
            return false;
        t = move(m_queue[m_head & m_mask]);
        // 执行析构函数，防止没有移动赋值函数,调用拷贝赋值函数后不释放queue的对象.如果对象有指针成员会造成内存泄露.
        ((T *) (&m_queue[m_head & m_mask]))->~T();
        m_head++;
    }
    m_notFullCv.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value, bool>::type SyncRingArrayV1<T>::pop(T *t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notEmptyMt);
        if(!m_notEmptyCv.wait_for(locker, duration, [this] {return m_head != m_tail;}))
            return false;
        *t = m_queue[m_head & m_mask];
        m_head++;
    }
    m_notFullCv.notify_one();
    return false;
}

template<typename T>
size_t SyncRingArrayV1<T>::size() {
    size_t len = m_tail - m_head;
    if(len > m_mask)
        len = m_tail - (m_head & m_mask) + m_size;
    return len;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value>::type SyncRingArrayV1<T>::clearData(void *isPoint) {
    while(m_head!=m_tail) {
        // 释放堆内存
        delete m_queue[m_head&m_mask];
        m_queue[m_head&m_mask] = nullptr;
        m_head++;
    }
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value>::type SyncRingArrayV1<T>::clearData(int isNotPoint) {
    // delete[] m_queue 会自动调用析构函数
}

template<typename T>
template<typename TT>
bool SyncRingArrayV1<T>::_push(TT &&t, microseconds duration) {
    {
        unique_lock<mutex> locker(m_notFullMt);
        if(!m_notFullCv.wait_for(locker, duration, [this]{return m_tail-m_head < m_mask;}))
            return false;
        new(&m_queue[m_tail & m_mask]) T{forward<T>(t)};
        m_tail++;
    }
    m_notEmptyCv.notify_one();
    return true;
}

#endif

#endif //C__11_TEST_SYNC_RING_ARRAY_V1_H
