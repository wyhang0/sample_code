//
// Created by root on 7/23/21.
//

#ifndef C__11_TEST_TIMECONSUMINGTASKTHREADS_H
#define C__11_TEST_TIMECONSUMINGTASKTHREADS_H

#include <functional>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

#include "./syncqueue/sync_ring_array_v2.h"

using namespace std;
using namespace chrono;

using TaskType = function<void()>;

template<typename SyncQueue=SyncRingArrayV2<TaskType>>
class TimeConsumingTaskThreads{
public:
    TimeConsumingTaskThreads(int threadCounts=thread::hardware_concurrency()==0 ? 2 : thread::hardware_concurrency())
            :threadCounts(threadCounts){
    }
    ~TimeConsumingTaskThreads(){
        stop();
    }

    TimeConsumingTaskThreads(const TimeConsumingTaskThreads &other)=delete;
    TimeConsumingTaskThreads(TimeConsumingTaskThreads&) = delete;

#if 1
    // 可调用对象包装器---std::function
    template<typename Fun>
    auto addTask(Fun &&fun)->future<typename result_of<Fun()>::type> {
        using R = typename result_of<Fun()>::type;
        auto task = make_shared<packaged_task<R()>>(forward<Fun>(fun));
        if(timeConsumingTaskQueue.push([=]{(*task)();})){
            notEmptyCv.notify_one();
            return (*task).get_future();
        }else{
            return {};
        }
    }
    // 接受packaged_task
    template<typename R>
    auto addTask(packaged_task<R()> &&packagedTask) -> future<R> {
        auto task = make_shared<packaged_task<R()>>(move(packagedTask));
        if(timeConsumingTaskQueue.push([=]{(*task)();})){
            notEmptyCv.notify_one();
            return (*task).get_future();
        }else{
            return {};
        }
    }

#else
    /**
     * 接受function, lambda, 普通函数, 伪函数(operator()重载)
     * 注意事项：
     * 1.伪函数 会有两次构造函数调用,不能修改原始对象的成员变量,不建议直接使用.可以封装为lambda使用
     * 2.f表达式的形参有引用时,执行完f引用变量改变的内容并不会体现在外部，因为执行bind绑定，f改变的是通过引用变量拷贝构造的对象。如果需要引用形参的变化，把形参设改为指针就可以了。
     * @tparam F
     * @tparam Args
     * @param f
     * @param args
     * @return
     */
    template<typename F, typename ...Args, typename enable_if<!is_member_function_pointer<F>::value>::type* = nullptr>
    auto addTask(F &&f, Args&&...args) -> future<typename result_of<F(Args...)>::type> {
        using Return_type = typename result_of<F(Args...)>::type;
        auto task = make_shared<packaged_task<Return_type()>>(bind(forward<F>(f), forward<Args>(args)...));
        if(timeConsumingTaskQueue.push([=]{(*task)();})){
            notEmptyCv.notify_one();
            return (*task).get_future();
        }else{
            return {};
        }
    }
    // 接受packaged_task
    template<typename R>
    auto addTask(packaged_task<R()> &&packagedTask) -> future<R> {
        auto task = make_shared<packaged_task<R()>>(move(packagedTask));
        if(timeConsumingTaskQueue.push([=]{(*task)();})){
            notEmptyCv.notify_one();
            return (*task).get_future();
        }else{
            return {};
        }
    }
    // 接受普通成员函数
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* f)(Args...), CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* &f)(Args...), CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    // 接受const成员函数
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* f)(Args...)const, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* &f)(Args...)const, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    // 接受volatile成员函数
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* f)(Args...)volatile, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* &f)(Args...)volatile, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    // 接受const volatile成员函数
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* f)(Args...)const volatile, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
    template<typename R, typename C, typename CP, typename ...Args>
    auto addTask(R(C::* &f)(Args...)const volatile, CP &&cp, Args&&...args) -> future<R> {
        return _add_member_function<R>(f, cp, args...);
    }
#endif

    void start(){
        lock_guard<std::mutex> lockGuard{mt};
        if(isRun)
            return;
        isRun = true;
        for (int i = 0; i < threadCounts; ++i) {
            threads.emplace_back(thread{[&]{
                function<void()> task;
                while(isRun) {
                    if(!timeConsumingTaskQueue.pop(task)){
                        unique_lock<mutex> locker{notEmptyMt};
                        notEmptyCv.wait_for(locker, restTime);
                        continue;
                    }
                    task();
                }
            }});
        }
    }

    void stop(){
        lock_guard<std::mutex> lockGuard{mt};
        if(!isRun)
            return;
        isRun = false;
        for (int i = 0; i < threadCounts; ++i) {
            threads[i].join();
        }
    }

    size_t size() {
        return timeConsumingTaskQueue.size();
    }

private:
    template<typename R, typename P, typename CP, typename ...Args>
    auto _add_member_function(P &&p, CP &&cp, Args&&...args) -> future<R> {
        auto task = make_shared<packaged_task<R()>>(bind(p, cp, forward<Args>(args)...));
        if(timeConsumingTaskQueue.push([=]{(*task)();})){
            notEmptyCv.notify_one();
            return (*task).get_future();
        }else{
            return {};
        }
    }

private:
    volatile bool isRun{false};
    int threadCounts{2};
    vector<thread> threads;
    mutex mt;
    SyncQueue timeConsumingTaskQueue;

    milliseconds restTime{600};
    mutex notEmptyMt;
    condition_variable notEmptyCv;
};

#endif //C__11_TEST_TIMECONSUMINGTASKTHREADS_H
