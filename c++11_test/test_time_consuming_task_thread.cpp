//
// Created by root on 7/22/21.
//

#include <iostream>

#include "./src/time_consuming_task_threads.h"

struct TK {
    int m_a;

    TK(int a){
        cout << "TK(int) " << this << endl;
        cout.flush();
        m_a = a;
    }
    TK(const TK &tk){
        cout << "TK(const TK &) " << this << endl;
        cout.flush();
        this->m_a = tk.m_a;
    }
    TK(TK &&tk){
        cout << "TK(TK &&) " << this << endl;
        cout.flush();
        this->m_a = tk.m_a;
    }
    ~TK(){
        cout << "~TK() " << this << endl;
        cout.flush();
    }
    TK& operator=(const TK &tk){
        if(&tk == this)
            return *this;
        cout << "operator= " << this << endl;
        cout.flush();
        this->m_a = tk.m_a;
        return *this;
    }
    int operator()(){
        this_thread::sleep_for(milliseconds{500});
        cout << "operator() " << this << endl;
        cout.flush();
        return m_a;
    }
    int operator()(int n){
        this_thread::sleep_for(milliseconds{500});
        cout << "operator(int) " << this << endl;
        cout.flush();
        m_a+=n;
        return m_a;
    }
    int f() const{
        this_thread::sleep_for(milliseconds{500});
        cout << "f() " << this << endl;
        cout.flush();
        return m_a;
    }
    int f(int i){
        this_thread::sleep_for(milliseconds{500});
        cout << "f(int) " << this << endl;
        cout.flush();
        m_a+=i;
        return m_a;
    }
    int f(int i, int j){
        this_thread::sleep_for(milliseconds{500});
        cout << "f(int,int) " << this << endl;
        cout.flush();
        m_a+=i+j;
        return m_a;
    }
};

int add_one(int n){
    this_thread::sleep_for(milliseconds{500});
    return n+1;
}

int main(){
    TimeConsumingTaskThreads<> consumingTaskThreads;
    consumingTaskThreads.start();

    vector<shared_future<int>> futures;
    TK tk{4};

    // 1.普通函数
    using F = int(int);
    using FP = int(*)(int);
    using FR = int(&)(int);
    F *f = add_one;
    FP fp = add_one;
    FR fr = add_one;
    futures.emplace_back(consumingTaskThreads.addTask([]{ return add_one(1);}));
    // 2.function
    futures.emplace_back(consumingTaskThreads.addTask(function<int()>{[]{return add_one(2);}}));
    // 3.lambda
    futures.emplace_back(consumingTaskThreads.addTask([]{return add_one(3);}));
    // 4.伪函数 有两次构造函数调用,不能修改原始对象的成员变量,不建议使用
    /**
     * TK(int) 0x7ffca0a8eed0 -> TK tk{5}调用一次TK构造函数
     * TK(const TK &) 0x7ffca0a8ee40 -> 构建bind对象调用一次TK拷贝构造函数
     * TK(const TK &) 0x7ffca0a8ee70 -> 构建packaged_task对象调用一次TK拷贝构造函数
     */
//    futures.emplace_back(consumingTaskThreads.addTask(tk));
    /**
     * 封装为lambda
     */
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk(1);}));
    // 5.packaged_task
    futures.emplace_back(consumingTaskThreads.addTask(packaged_task<int()>{[&]{return tk(1);}}));
    // 6. 成员函数
    using M = int(TK::*)(int);
    using MM = int(TK::* &)(int);
    M m = &TK::f;
    MM mm = m;
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk.f();}));
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk.f(1);}));
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk.f(0, 1);}));
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk();}));
    futures.emplace_back(consumingTaskThreads.addTask([&]{return tk(1);}));

    for (auto &item:futures) {
        cout << item.get() << endl;
    }

    return 0;
}

