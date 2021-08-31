//
// Created by root on 2021/7/19.
//

#ifndef C__11_TEST_SYNC_RING_ARRAY_V2_H
#define C__11_TEST_SYNC_RING_ARRAY_V2_H

#include "sync_queue.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <type_traits>

using namespace std;
using namespace chrono;

/**
 * v2
 * 读写线程并发执行，读线程并发执行，写线程并发执行。N:M生产消费者模型最快。
 * 队列大小建议>max(N,M),防止自旋获取值或者自旋设置值时，cpu空跑，浪费资源。
 *
 * 实现大致思路：
 * 该队列结合atomic与mutex,atomic用于获取队列节点(Node)位置是多线程并发的,对节点(Node)进行取值与赋值操作是通过锁同步的.把锁的粒度放到节点上,降低了实现的难易度.
 *
 * 缺点：
 * 1.queue空或者满时，自旋操作queue会有cpu空跑。
 * 2.queue大小<max(N,M)时，自旋操作queue会有cpu空跑。
 * 3.任务耗时较短时，cpu占用率偏高。
 *
 * 当队列操作速度比较慢时，可以使用，未优化的情况可以不考虑。
 *
 * @tparam T
 */
#if 1 // 区别是使用了aligned_storage，防止构造数组时执行构造函数
template<typename T>
class SyncRingArrayV2 : public SyncQueue<T>{
public:
    /**
     *
     * @param sleepInterval
     * 该值对性能影响有直观影响，可根据性能要求进行调整
     * 值越小速度越快，cpu占用率越高
     * @param size
     */
    explicit SyncRingArrayV2(size_t size=256, microseconds sleepInterval=microseconds{100});
    ~SyncRingArrayV2();
    SyncRingArrayV2(const SyncRingArrayV2<T> &)=delete;

    bool push(T &&t, microseconds duration=microseconds{250});
    bool push(const T &t, microseconds duration=microseconds{250});

    template<typename ...Args>
    bool emplace(microseconds duration, Args&&...args);

    template<typename TT=T, typename enable_if<!is_pointer<TT>::value>::type* = nullptr>
    bool pop(T &t, microseconds duration=microseconds{250});
    template<typename TT=T, typename enable_if<is_pointer<TT>::value>::type* = nullptr>
    bool pop(T *t, microseconds duration=microseconds{250});

    size_t size();

private:
    template<typename TT>
    bool _push(TT &&t, microseconds duration=microseconds{250});
    pair<bool, size_t> getPositionForPut(microseconds duration);
    pair<bool, size_t> getPositionForPop(microseconds duration);
    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value>::type clearData(void *isPoint=nullptr);
    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value>::type clearData(int isNotPoint=1);

private:
    struct Node{
        condition_variable isTrue;
        condition_variable isFalse;
        mutex mt;
        typename aligned_storage<sizeof(T), alignof(T)>::type t;
        bool flag{false};
    };
    Node *m_queue;
    size_t m_size;
    size_t m_mask;
    atomic<size_t> m_head{0};
    atomic<size_t> m_tail{0};

    microseconds m_sleepInterval;
    //Node节点条件变量的重试时间间隔,该值对性能几乎没有影响.
    microseconds m_condition_sleep{100};
};

template<typename T>
SyncRingArrayV2<T>::SyncRingArrayV2(size_t size, microseconds sleepInterval) {
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
        // 2.接近size值的2次幂值
        t1 = t1<<1;
        size = t1;
    }
    m_queue = new Node[size];
    m_size = size;
    m_mask = m_size-1;
    m_sleepInterval = sleepInterval;
}

template<typename T>
SyncRingArrayV2<T>::~SyncRingArrayV2() {
    if(m_queue){
        clearData();
        delete[] m_queue;
        m_queue = nullptr;
    }
}

template<typename T>
bool SyncRingArrayV2<T>::push(T &&t, microseconds duration) {
    return _push(move(t), duration);
}

template<typename T>
bool SyncRingArrayV2<T>::push(const T &t, microseconds duration) {
    return _push(t, duration);
}

template<typename T>
template<typename ...Args>
bool SyncRingArrayV2<T>::emplace(microseconds duration, Args &&... args) {
    // 1.高并发获取节点位置
    auto result = getPositionForPut(duration);
    if(!result.first)
        return false;
    // 2.同步设置节点值.
    size_t tail = result.second;
    {
        unique_lock<mutex> locker(m_queue[tail&m_mask].mt);
        while(!m_queue[tail&m_mask].isFalse.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[tail & m_mask].flag == false;}));
        new (&m_queue[tail&m_mask].t) T{forward<Args>(args)...};
        m_queue[tail&m_mask].flag = true;
    }
    // 3.唤醒其他(pop)线程
    m_queue[tail&m_mask].isTrue.notify_one();
    return true;
}

template<typename T>
template<typename TT, typename enable_if<!is_pointer<TT>::value>::type*>
bool SyncRingArrayV2<T>::pop(T &t, microseconds duration){
    // 1.高并发获取节点位置
    auto result = getPositionForPop(duration);
    if(!result.first)
        return false;
    // 2.同步获取节点值.
    size_t head = result.second;
    {
        unique_lock<mutex> locker(m_queue[head&m_mask].mt);
        while(!m_queue[head&m_mask].isTrue.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[head & m_mask].flag == true;}));
        t = move(*reinterpret_cast<T*>(&m_queue[head&m_mask].t));
        // 执行析构函数，防止没有移动赋值函数,调用拷贝赋值函数后不释放queue的对象.如果对象有指针成员会造成内存泄露.
        reinterpret_cast<T*>(&m_queue[head&m_mask].t)->~T();
        m_queue[head&m_mask].flag = false;
    }
    m_queue[head&m_mask].isFalse.notify_one();

    return true;
}

template<typename T>
template<typename TT, typename enable_if<is_pointer<TT>::value>::type*>
bool SyncRingArrayV2<T>::pop(T *t, microseconds duration){
    // 1.高并发获取节点位置
    auto result = getPositionForPop(duration);
    if(!result.first)
        return false;
    // 2.同步获取节点值.
    size_t head = result.second;
    {
        unique_lock<mutex> locker(m_queue[head&m_mask].mt);
        while(!m_queue[head&m_mask].isTrue.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[head & m_mask].flag == true;}));
        *t = *reinterpret_cast<T*>(&m_queue[head&m_mask].t);
        m_queue[head&m_mask].flag = false;
    }
    m_queue[head&m_mask].isFalse.notify_one();

    return true;
}

template<typename T>
size_t SyncRingArrayV2<T>::size(){
    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    size_t len = tail - head;
    if(len > m_mask)
        len = tail-(head&m_mask)+m_size;
    return len;
}

template<typename T>
pair<bool, size_t> SyncRingArrayV2<T>::getPositionForPut(microseconds duration) {
    auto start = system_clock::now();

    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    while(tail-head>=m_mask || !m_tail.compare_exchange_weak(tail, tail+1, memory_order_relaxed)){

        //此处休眠值大小对性能影响很大
        this_thread::sleep_for(m_sleepInterval);

        head = m_head.load(memory_order_relaxed);
        tail = m_tail.load(memory_order_relaxed);
        if(duration.count() > 0){
            if(duration_cast<microseconds>(system_clock::now()-start) > duration)
                return {false, 0};
        }
    }
    return {true, tail};
}

template<typename T>
pair<bool, size_t> SyncRingArrayV2<T>::getPositionForPop(microseconds duration) {
    auto start = system_clock::now();
    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    while(tail==head || !m_head.compare_exchange_weak(head, head+1, memory_order_relaxed)){

        //此处休眠值大小对性能影响很大
        this_thread::sleep_for(m_sleepInterval);

        head = m_head.load(memory_order_relaxed);
        tail = m_tail.load(memory_order_relaxed);
        if(duration.count()>0){
            if(duration_cast<microseconds>(system_clock::now()-start) > duration)
                return{false, 0};
        }
    }
    return {true, head};
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value>::type SyncRingArrayV2<T>::clearData(void *isPoint) {
    while(m_head!=m_tail) {
        // 释放堆内存
        delete *(reinterpret_cast<T*>(&m_queue[m_head&m_mask].t));
        m_queue[m_head&m_mask].flag = false;
        m_head++;
    }
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value>::type SyncRingArrayV2<T>::clearData(int type) {
    while(m_head!=m_tail) {
        // 执行析构函数，防止对象有指针成员会造成内存泄露.
        reinterpret_cast<T*>(&m_queue[m_head&m_mask].t)->~T();
        m_queue[m_head&m_mask].flag = false;
        m_head++;
    }
}

template<typename T>
template<typename TT>
bool SyncRingArrayV2<T>::_push(TT &&t, microseconds duration) {
    // 1.高并发获取节点位置
    auto result = getPositionForPut(duration);
    if(!result.first)
        return false;
    // 2.同步设置节点值.
    size_t tail = result.second;
    {
        unique_lock<mutex> locker(m_queue[tail&m_mask].mt);
        while(!m_queue[tail&m_mask].isFalse.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[tail & m_mask].flag == false;}));
        new (&m_queue[tail&m_mask].t) T{t};
        m_queue[tail&m_mask].flag = true;
    }
    // 3.唤醒其他(pop)线程
    m_queue[tail&m_mask].isTrue.notify_one();
    return true;
}
#else
template<typename T>
class SyncRingArrayV2 : public SyncQueue<T>{
public:
    /**
     *
     * @param sleepInterval
     * 该值对性能影响有直观影响，可根据性能要求进行调整
     * 值越小速度越快，cpu占用率越高
     * @param size
     */
    explicit SyncRingArrayV2(size_t size=1024, microseconds sleepInterval=microseconds{100});
    ~SyncRingArrayV2();
    SyncRingArrayV2(const SyncRingArrayV2<T> &)=delete;

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
    pair<bool, size_t> getPositionForPut(microseconds duration);
    pair<bool, size_t> getPositionForPop(microseconds duration);

    template<typename TT=T>
    typename enable_if<is_pointer<TT>::value>::type clearData(void *isPoint=nullptr);
    template<typename TT=T>
    typename enable_if<!is_pointer<TT>::value>::type clearData(int isNotPoint=1);

private:
    struct Node{
        T t;
        bool flag{false};
        mutex mt;
        condition_variable isTrue;
        condition_variable isFalse;
    };
    Node *m_queue;
    size_t m_size;
    size_t m_mask;
    atomic<size_t> m_head{0};
    atomic<size_t> m_tail{0};

    microseconds m_sleepInterval;
    //Node节点条件变量的重试时间间隔,该值对性能几乎没有影响.
    microseconds m_condition_sleep{100};
};

template<typename T>
SyncRingArrayV2<T>::SyncRingArrayV2(size_t size, microseconds sleepInterval) {
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
        // 2.接近size值的2次幂值
        t1 = t1<<1;
        size = t1;
    }
    m_queue = new Node[size];
    m_size = size;
    m_mask = m_size-1;
    m_sleepInterval = sleepInterval;
}

template<typename T>
SyncRingArrayV2<T>::~SyncRingArrayV2() {
    if(m_queue){
        clearData();
        delete[] m_queue;
        m_queue = nullptr;
    }
}

template<typename T>
bool SyncRingArrayV2<T>::push(T &&t, microseconds duration) {
    return _push(move(t), duration);
}

template<typename T>
bool SyncRingArrayV2<T>::push(const T &t, microseconds duration) {
    return _push(t, duration);
}

template<typename T>
template<typename ...Args>
bool SyncRingArrayV2<T>::emplace(microseconds duration, Args &&... args) {
    // 1.高并发获取节点位置
    auto result = getPositionForPut(duration);
    if(!result.first)
        return false;
    // 2.同步设置节点值.
    size_t tail = result.second;
    {
        unique_lock<mutex> locker(m_queue[tail&m_mask].mt);
        while(!m_queue[tail&m_mask].isFalse.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[tail & m_mask].flag == false;}));
        new (&m_queue[tail&m_mask].t) T{forward<Args>(args)...};
        m_queue[tail&m_mask].flag = true;
    }
    // 3.唤醒其他(pop)线程
    m_queue[tail&m_mask].isTrue.notify_one();
    return true;
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value, bool>::type SyncRingArrayV2<T>::pop(T &t, microseconds duration){
    // 1.高并发获取节点位置
    auto result = getPositionForPop(duration);
    if(!result.first)
        return false;
    // 2.同步获取节点值.
    size_t head = result.second;
    {
        unique_lock<mutex> locker(m_queue[head&m_mask].mt);
        while(!m_queue[head&m_mask].isTrue.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[head & m_mask].flag == true;}));
        t = move(m_queue[head&m_mask].t);
        // 执行析构函数，防止没有移动赋值函数,调用拷贝赋值函数后不释放queue的对象.如果对象有指针成员会造成内存泄露.
        ((T *) (&m_queue[head&m_mask]))->~T();
        m_queue[head&m_mask].flag = false;
    }
    m_queue[head&m_mask].isFalse.notify_one();

    return true;
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value, bool>::type SyncRingArrayV2<T>::pop(T *t, microseconds duration){
    // 1.高并发获取节点位置
    auto result = getPositionForPop(duration);
    if(!result.first)
        return false;
    // 2.同步获取节点值.
    size_t head = result.second;
    {
        unique_lock<mutex> locker(m_queue[head&m_mask].mt);
        while(!m_queue[head&m_mask].isTrue.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[head & m_mask].flag == true;}));
        *t = m_queue[head&m_mask].t;
        m_queue[head&m_mask].t = nullptr;
        m_queue[head&m_mask].flag = false;
    }
    m_queue[head&m_mask].isFalse.notify_one();

    return true;
}

template<typename T>
size_t SyncRingArrayV2<T>::size(){
    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    size_t len = tail - head;
    if(len > m_mask)
        len = tail-(head&m_mask)+m_size;
    return len;
}

template<typename T>
pair<bool, size_t> SyncRingArrayV2<T>::getPositionForPut(microseconds duration) {
    auto start = system_clock::now();

    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    while(tail-head>=m_mask || !m_tail.compare_exchange_weak(tail, tail+1, memory_order_relaxed)){

        //此处休眠值大小对性能影响很大
        this_thread::sleep_for(m_sleepInterval);

        head = m_head.load(memory_order_relaxed);
        tail = m_tail.load(memory_order_relaxed);
        if(duration.count() > 0){
            if(duration_cast<microseconds>(system_clock::now()-start) > duration)
                return {false, 0};
        }
    }
    return {true, tail};
}

template<typename T>
pair<bool, size_t> SyncRingArrayV2<T>::getPositionForPop(microseconds duration) {
    auto start = system_clock::now();
    size_t head = m_head.load(memory_order_relaxed);
    size_t tail = m_tail.load(memory_order_relaxed);
    while(tail==head || !m_head.compare_exchange_weak(head, head+1, memory_order_relaxed)){

        //此处休眠值大小对性能影响很大
        this_thread::sleep_for(m_sleepInterval);

        head = m_head.load(memory_order_relaxed);
        tail = m_tail.load(memory_order_relaxed);
        if(duration.count()>0){
            if(duration_cast<microseconds>(system_clock::now()-start) > duration)
                return{false, 0};
        }
    }
    return {true, head};
}

template<typename T>
template<typename TT>
typename enable_if<is_pointer<TT>::value>::type SyncRingArrayV2<T>::clearData(void *isPoint) {
    while(m_head!=m_tail) {
        // 释放堆内存
        delete m_queue[m_head&m_mask].t;
        m_queue[m_head&m_mask].flag = false;
        m_head++;
    }
}

template<typename T>
template<typename TT>
typename enable_if<!is_pointer<TT>::value>::type SyncRingArrayV2<T>::clearData(int type) {
    // delete[] m_queue 会自动调用析构函数
}

template<typename T>
template<typename TT>
bool SyncRingArrayV2<T>::_push(TT &&t, microseconds duration) {
    // 1.高并发获取节点位置
    auto result = getPositionForPut(duration);
    if(!result.first)
        return false;
    // 2.同步设置节点值.
    size_t tail = result.second;
    {
        unique_lock<mutex> locker(m_queue[tail&m_mask].mt);
        while(!m_queue[tail&m_mask].isFalse.wait_for(locker, m_condition_sleep, [&,this]{return m_queue[tail & m_mask].flag == false;}));
        new(&m_queue[tail&m_mask].t) T{forward<T>(t)};
        m_queue[tail&m_mask].flag = true;
    }
    // 3.唤醒其他(pop)线程
    m_queue[tail&m_mask].isTrue.notify_one();
    return true;
}
#endif

#endif //C__11_TEST_SYNC_RING_ARRAY_V2_H
