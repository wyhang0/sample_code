//
// Created by root on 8/3/21.
//

#include <iostream>

#include "./src/singleton.h"
#include "./src/student.h"

using namespace std;

int main(){
    // 第一次调用可以使用有参构造函数初始化 instance
    Singleton<Student>::instance("wy", 18, 100);
    // 第一次调用可以使用无参构造函数初始化 instance
    Singleton<int>::instance(100);

    // 获取instance实例
    cout << *Singleton<Student>::instance() << endl;
    cout << *Singleton<int>::instance() << endl;
    cout << *Singleton<double>::instance() << endl;

    return 0;
}

