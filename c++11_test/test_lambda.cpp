//
// Created by root on 8/2/21.
//
#include <iostream>
#include <functional>

using namespace std;

int main(){
    {
        int i = 1;
        auto l1 = [=]() { cout << i << endl; };
        i = 2;
        auto l2 = [=]() { cout << i << endl; };
        i = 3;
        auto l3 = [=]() { cout << i << endl; };
        l1();
        l2();
        l3();
    }
    {
        int i = 1;
        auto l1 = [&]() { cout << i << endl; };
        i = 2;
        auto l2 = [&]() { cout << i << endl; };
        i = 3;
        auto l3 = [&]() { cout << i << endl; };
        l1();
        l2();
        l3();
    }
    {
        auto f = (int(*)(int))[](int i){ return i; };
        f(1);
    }
    {
        auto f = (function<int(int)>)[](int i){ return i; };
        f(1);
    }
    return 0;
}

