//
// Created by root on 8/10/21.
//

#include <iostream>

#include "./src/singleton.h"
#include "./src/ioc_container.h"
#include "./src/student.h"

using namespace std;

struct ICar{
    virtual ~ICar(){}
    virtual void fun()=0;
};
struct Car : ICar{
    Car(){
        cout << "Car()" << endl;
        cout.flush();
    }
    Car(int){
        cout << "Car(int)" << endl;
        cout.flush();
    }
    Car(int, int){
        cout << "Car(int, int)" << endl;
        cout.flush();
    }
    void fun(){
        cout << "Car::fun" << endl;
    }
};
struct Bus : ICar{
    Bus(){
        cout << "Bus()" << endl;
        cout.flush();
    }
    Bus(int){
        cout << "Bus(int)" << endl;
        cout.flush();
    }
    Bus(int, int){
        cout << "Bus(int, int)" << endl;
        cout.flush();
    }
    void fun(){
        cout << "Bus::fun" << endl;
        cout.flush();
    }
};

struct IA{
    virtual ~IA(){}
    virtual void a()=0;
};
struct A : IA{
    A(ICar *car) : m_car(car){
        cout << "A(ICar)" << endl;
        cout.flush();
    }
    A(int , ICar *car) : m_car(car){
        cout << "A(int, ICar)" << endl;
        cout.flush();
    }
    A(int, int, ICar *car) : m_car(car){
        cout << "A(int, int, ICar)" << endl;
        cout.flush();
    }
    // *******************************************************
    A(Student&, ICar *car) : m_car(car){
        cout << "A(Student&, int, ICar)" << endl;
        cout.flush();
    }
    // *******************************************************
    A(Student&&, ICar *car) : m_car(car){
        cout << "A(Student&&, int, ICar)" << endl;
        cout.flush();
    }
    ~A(){
        if(m_car){
            delete m_car;
            m_car = nullptr;
        }
    }
    void a(){
        if(m_car)
            m_car->fun();
    }
private:
    ICar *m_car;
};

struct B{
    B(IA *a) : m_a(a){
        cout << "B(IA)" << endl;
        cout.flush();
    }
    B(int , IA *a) : m_a(a){
        cout << "B(int, IA)" << endl;
        cout.flush();
    }
    B(int, int, IA *a) : m_a(a){
        cout << "B(int, int, IA)" << endl;
        cout.flush();
    }
    ~B(){
        if(m_a){
            delete m_a;
            m_a = nullptr;
        }
    }
    void b(){
        if(m_a)
            m_a->a();
    }
private:
    IA *m_a;
};

int main(){
    auto iocContainer = Singleton<IocContainer>::instance();

    // 继承关系
    iocContainer->registerType<ICar, Car>("icar1");
    iocContainer->registerType<ICar, Car, int>("icar2");
    iocContainer->registerType<ICar, Car, int, int>("icar3");
    iocContainer->registerType<ICar, Bus>("ibus1");
    iocContainer->registerType<ICar, Bus, int>("ibus2");
    iocContainer->registerType<ICar, Bus, int, int>("ibus3");
    {
        auto icar = iocContainer->resolveShared<ICar>("icar1");
        icar->fun();
        icar = iocContainer->resolveShared<ICar>("icar2", 1);
        icar->fun();
        icar = iocContainer->resolveShared<ICar>("icar3", 1, 1);
        icar->fun();
        icar = iocContainer->resolveShared<ICar>("ibus1");
        icar->fun();
        icar = iocContainer->resolveShared<ICar>("ibus2", 1);
        icar->fun();
        icar = iocContainer->resolveShared<ICar>("ibus3", 1, 1);
        icar->fun();
        cout << endl;
        cout.flush();
    }
    // 普通对象
    iocContainer->registerSimple<Car>("car1");
    iocContainer->registerSimple<Car, int>("car2");
    iocContainer->registerSimple<Car, int, int>("car3");
    {
        auto icar = iocContainer->resolveShared<Car>("car1");
        icar->fun();
        icar = iocContainer->resolveShared<Car>("car2", 1);
        icar->fun();
        icar = iocContainer->resolveShared<Car>("car3", 1, 1);
        icar->fun();
    }
    // 依赖关系 1
    iocContainer->registerType<A, Car>("a1");
    iocContainer->registerType<A, Car, int>("a2");
    iocContainer->registerType<A, Car, int, int>("a3");
    {
        auto a = iocContainer->resolveShared<A>("a1");
        a->a();
        a = iocContainer->resolveShared<A>("a2", 1);
        a->a();
        a = iocContainer->resolveShared<A>("a3", 1, 1);
        a->a();
    }
    // 依赖关系 2 **********************************************************************
    iocContainer->registerType<A, Car>("ac1", "car1");
    iocContainer->registerType<A, Car>("ac2", "car2", 1);
    iocContainer->registerType<A, Car>("ac3", "car3", 1, 1);
    iocContainer->registerType<A, Car, int>("ac4", "car1");
    iocContainer->registerType<A, Car, int>("ac5", "car2", 1);
    iocContainer->registerType<A, Car, int>("ac6", "car3", 1, 1);
    {
        auto a = iocContainer->resolveShared<A>("ac1");
        a->a();
        a = iocContainer->resolveShared<A>("ac2", 1);
        a->a();
        a = iocContainer->resolveShared<A>("ac3", 1, 1);
        a->a();
        a = iocContainer->resolveShared<A>("ac4", 1);
        a->a();
        a = iocContainer->resolveShared<A>("ac5", 1, 1);
        a->a();
        a = iocContainer->resolveShared<A>("ac6", 1, 1, 1);
        a->a();
    }
    // 依赖关系 2 **********************************************************************
    Student s;
    iocContainer->registerType<A, Car, Student&>("lvalue");
    iocContainer->registerType<A, Car, Student>("rvalue");
    iocContainer->registerType<B, A>("b1", "a1");
    iocContainer->registerType<B, A>("b2", "a2", 1);
    iocContainer->registerType<B, A>("b3", "a3", 1, 1);
    iocContainer->registerType<B, A, int>("b4", "ac5", 1, 1);
    iocContainer->registerType<B, A, int>("b5", "ac6", 1, 1, 1);
    iocContainer->registerType<B, A, int>("b6", "lvalue", *((Student*)0));
    iocContainer->registerType<B, A, int>("b7", "lvalue", s);
    iocContainer->registerType<B, A, int>("b8", "rvalue", Student{});
    {
        auto b = iocContainer->resolveShared<B>("b1");
        b->b();
        b = iocContainer->resolveShared<B>("b2", 1);
        b->b();
        b = iocContainer->resolveShared<B>("b3", 1, 1);
        b->b();
        b = iocContainer->resolveShared<B>("b4", 1, 1, 1);
        b->b();
        b = iocContainer->resolveShared<B>("b5", 1, 1, 1, 1);
        b->b();
        b = iocContainer->resolveShared<B>("b6", 1, s);
        b->b();
        b = iocContainer->resolveShared<B>("b7", 1, s);
        b->b();
        b = iocContainer->resolveShared<B>("b8", 1, Student{});
        b->b();
    }

    return 0;
}
