//
// Created by root on 8/11/21.
//

#ifndef C__11_TEST_MESSAGE_BUS_H
#define C__11_TEST_MESSAGE_BUS_H

#include <map>
#include <functional>
#include <mutex>
#include <vector>
#include <algorithm>

#include "./non_copyable.h"
#include "./any.h"
#include "./function_traits.h"
#include "./time_consuming_task_threads.h"
#include "./syncqueue/sync_ring_array_v1.h"

using namespace std;

struct MessageBusModel;
/**
 * 实现模块间的高内聚低耦合,使用方式参考硬件总线.
 * 消息异步执行,参数不要使用引用,用指针代替,其他使用右值.消息异步执行保证函数的可重入性.
 * 参数用引用会执行对象的构造函数,并且接受者改变对象属性改变的是临时对象的,对发送者不可见.
 *　注册消息有两种方式。
 * １.全局对象　shared_ptr<T> obj
 * bus.attach([shared_ptr<T> obj](Args&&...args){obj.something(forward<Args>(args)...);}, topic);
 * 2.局部对象 class Obj : MessageBusModel { void f(); }
 * 当局部对象注册消息后,必须执行obj.removeMessage()去移除消息,局部对象才会析构.
 */
class MessageBus : NonCopyable{
public:
    MessageBus(){
        m_threads.start();
    }
    ~MessageBus(){
        while (m_threads.size() != 0){
            cout << "wait for ~MessageBus()" << endl;
            this_thread::sleep_for(milliseconds{100});
        }
        m_threads.stop();
    }
    /**
     *
     * @tparam Fun
     * Fun为lambda表达式.　
     * eg:
     * MessageBus g_messageBus;
     * struct A{
     *      A(){
     *          g_messageBus.attach([this](const int &i){something(i);});
     *          g_messageBus.attach([](const int &i){f(i);});
     *      }
     *      void something(const int &i){}
     *      static void f(const int &i){}
     * };
     * @param fun
     * @param topic
     */
    template<typename Fun>
    pair<string, size_t> attach(Fun &&fun, const string &topic=""){
        using Function = typename function_traits<Fun>::stl_function;
        string msgType = topic + typeid(Function).name();

        lock_guard<mutex> lockGuard{m_mutex};
        m_map.template emplace(msgType, Message<Function>{m_id, (Function)fun});
        return {move(msgType), m_id++};
    }

    template<typename R=void, typename ...Args>
    void remove(const string &topic=""){
        using Function = function<R(Args...)>;
        string msgType = topic + typeid(Function).name();

        lock_guard<mutex> lockGuard{m_mutex};
        auto range = m_map.equal_range(msgType);
        m_map.erase(range.first, range.second);
    }

    void remove(size_t id, const string &msgType=""){
        lock_guard<mutex> lockGuard{m_mutex};
        auto range = m_map.equal_range(msgType);
        m_map.erase(find_if(range.first, range.second, [id](iterator_traits<decltype(range.first)>::reference item){
            return *((size_t*)item.second.point_cast())==id;
        }));
    }

    void removeAll(){
        lock_guard<mutex> lockGuard{m_mutex};
        m_map.clear();
    }

    template<typename R=void, typename ...Args>
    void syncSendReq(const string &topic="", Args&&...args){
        using Function = function<R(Args...)>;
        string msgType = topic + typeid(Function).name();

        vector<Function> vec;
        {
            lock_guard<mutex> lockGuard{m_mutex};
            auto range = m_map.equal_range(msgType);
            for_each(range.first, range.second, [&](typename iterator_traits<decltype(range.first)>::reference item){
                vec.emplace_back(item.second.template any_cast<Message<Function>>().message);
            });
        }
        for (auto &item:vec) {
            item(forward<Args>(args)...);
        }
    }

    /**
     * 异步保证接收者对象的生命周期大于MessageBus对象的生命周期
     * @tparam R
     * @tparam Args
     * @param args
     * @param topic
     */
    template<typename R=void, typename ...Args>
    void asyncSendReq(const string &topic="", Args &&...args){
        using Function = function<R(Args...)>;
        string msgType = topic + typeid(Function).name();

        vector<Function> vec;
        {
            lock_guard<mutex> lockGuard{m_mutex};
            auto range = m_map.equal_range(msgType);
            for_each(range.first, range.second, [&](typename iterator_traits<decltype(range.first)>::reference item){
                vec.emplace_back(item.second.template any_cast<Message<Function>>().message);
            });
        }
        for (auto &item:vec) {
            TaskType task{bind(item, forward<Args>(args)...)};
            future<R> r;
            while (!r.valid()){
                r = m_threads.addTask(task);
            }
        }
    }

private:
    template<typename T>
    struct Message{
        size_t id;
        T message;
    };
private:
    multimap<string, Any> m_map;
    size_t m_id{0};

    TimeConsumingTaskThreads<SyncRingArrayV1<TaskType>> m_threads{3};
    mutex m_mutex;
};

struct MessageBusModel : NonCopyable{
    virtual ~MessageBusModel(){
        waitForRemoveMessage();
    }

    template<typename Fun>
    void registerMessage(MessageBus *bus, Fun &&func, const string &topic) {
        lock_guard<mutex> lockGuard{m_mutex};
        m_messages.emplace(bus->attach(move(func), topic));
    }

    void removeMessage(MessageBus *bus) {
        lock_guard<mutex> lockGuard{m_mutex};
        if(m_messages.empty())
            return;
        for_each(m_messages.begin(), m_messages.end(), [bus](typename iterator_traits<decltype(m_messages.begin())>::reference item){
            bus->remove(item.second, item.first);
        });
        m_messages.clear();
    }

protected:
    void waitForRemoveMessage(){
        while(!m_messages.empty()){
            cout << "wait for ~MessageBusModel()" << endl;
            this_thread::sleep_for(milliseconds{100});
        }
    }

protected:
    mutex m_mutex;
    multimap<string, size_t> m_messages;
};

#endif //C__11_TEST_MESSAGE_BUS_H
