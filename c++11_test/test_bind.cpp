//
// Created by root on 8/10/21.
//
#include <iostream>
#include <functional>

#include "./src/student.h"

using namespace std;

void f1(Student &s){

}
void f2(Student &&s){

}
void f3(Student s){

}

void b(){
    cout << "b()" << endl;
}
void b(int i){
    cout << "b(int)" << endl;
}
void b(int i, double j){
    cout << "b(int, double)" << endl;
}
void c(){
    cout << "c()" << endl;
}
void c(int i){
    cout << "c(int)" << endl;
}
void c(int i, double j){
    cout << "c(int, double)" << endl;
}
template<typename ...BArgs, typename ...CArgs>
void d(BArgs&&...bArgs, CArgs&&...cArgs){
    b(forward<BArgs>(bArgs)...);
    c(forward<CArgs>(cArgs)...);
}

int main(){
    Student s{R"(wy)", 18, 100};
    // 执行Student拷贝构造
    auto a1 = bind(f1, s);
    // 使用Student的引用
    auto a2 = bind(f1, ref(s));
    // 执行Student拷贝构造
    auto a3 = bind(f2, s);
    // 把s变成右值,再执行Student移动构造
    auto a4 = bind(f2, move(s));
    // 执行Student构造函数,再执行Student移动构造
    auto a5 = bind(f2, Student{R"(wy)", 18, 100});
    // 执行Student拷贝构造
    auto a6 = bind(f3, s);

    d<int>(1, 1, 1);

    return 0;
}
