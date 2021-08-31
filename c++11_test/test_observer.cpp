//
// Created by root on 8/3/21.
//
#include <iostream>

#include "./src/events.h"

using namespace std;

struct A{
    ~A(){
        cout << "~A()" << this << endl;
    }
    int count=0;
    void print(int i, int j){
        cout << "A::print() " << i << ", " << j << ", " << count++ << this << endl;
    }
    void operator()(int i, int j){
        cout << "A::operator() " << i << ", " << j << ", " << count++ << this << endl;
    }
};
void print(int i, int j){
    static int count = 0;
    cout << "print() " << i << ", " << j << ", " << count++ << endl;
}

struct Data{
    int i, j;
    Events<void(int,int)> event;

    void reset(int i, int j){
        this->i=i;
        this->j=j;
        this->event.notify(this->i, this->j);
    }
};

int main(){
    // 以函数方式注册观察者
    {
        Events<void(int,int)> event;
        auto key = event.connection(print);
        event.notify(1, 2);
        event.notify(1, 2);
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    // lambda注册观察者
    {
        Events<void(int,int)> event;
        A a;
        // 保证event.notify的时候，对象a没有析构，否则行为未定义。可使用智能指针代替
        auto lambdakey = event.connection([&a](int i, int j) {
            a.print(i, j);
        });
        event.notify(1, 2);
        event.notify(1, 2);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    // 对象成员函数注册
    {
        Events<void(int,int)> event;
        A a;
        // 保证event.notify的时候，对象a没有析构，否则行为未定义。可使用智能指针代替
        event.connection(bind(&A::print, &a, placeholders::_1, placeholders::_2));
        event.notify(1, 2);
        event.notify(1, 2);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    // 伪函数注册
    {
        // 错误方式
        Events<void(int,int)> event;
        A a;
        event.connection(a);
        event.notify(1, 2);
        event.notify(1, 2);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    {
        // 正确方式
        Events<void(int,int)> event;
        A a;
        // 保证event.notify的时候，对象a没有析构，否则行为未定义。可使用智能指针代替
        event.connection(bind(&A::operator(), &a, placeholders::_1, placeholders::_2));
        event.notify(1, 2);
        event.notify(1, 2);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    {
        // 正确方式
        Events<void(int,int)> event;
        A a;
        // 保证event.notify的时候，对象a没有析构，否则行为未定义。可使用智能指针代替
        event.connection([&a](int i, int j) {
            a(i, j);
        });
        event.notify(1, 2);
        event.notify(1, 2);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();
    // test
    {
        Data data;
        A a;
        auto lambdakey = data.event.connection([&a](int i, int j) {
            a.print(i, j);
        });
        data.event.connection(bind(&A::print, &a, placeholders::_1, placeholders::_2));
        data.event.connection(bind(&A::operator(), &a, placeholders::_1, placeholders::_2));
        data.event.connection([&a](int i, int j) {
            a(i, j);
        });
        data.event.notify(1, 2);
        data.reset(3, 3);
        cout << a.count << endl;
    }
    cout << "---------------------------------------" << endl;
    cout.flush();

    return 0;
}
