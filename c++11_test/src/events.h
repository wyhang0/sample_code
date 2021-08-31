//
// Created by root on 8/3/21.
//

#ifndef C__11_TEST_EVENTS_H
#define C__11_TEST_EVENTS_H

#include <functional>
#include <map>

#include "non_copyable.h"

using namespace std;

template<typename T>
class Events;

template<typename R, typename ...Args>
class Events<R(Args...)> : NonCopyable {
public:
    // 注册观察者，返回观察者编号
    int connection(function<R(Args...)> &&fun){
        return assgin(move(fun));
    }
    int connection(const function<R(Args...)> &fun){
        return assgin(fun);
    }
    // 移除观察者
    void disconnection(int observerId){
        m_connections.erase(observerId);
    }
    // 通知所有观察者
    template<typename ...Parameters>
    void notify(Parameters&&...parameters){
        for (auto &item:m_connections) {
            item.second(forward<Parameters>(parameters)...);
        }
    }
private:
    template<typename F>
    int assgin(F &&f){
        int k = m_observerId++;
        m_connections.template emplace(k, forward<F>(f));
        return k;
    }
    // 观察者对应的编号
    int m_observerId{0};
    // 观察者列表
    map<int, function<R(Args...)>> m_connections;
};

#endif //C__11_TEST_EVENTS_H
