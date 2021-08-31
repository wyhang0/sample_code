//
// Created by root on 7/14/21.
//

#include <iostream>
#include <future>

using namespace std;

int main(){
    once_flag flag;

    auto fun = [&]{
        call_once(flag, []{ cout << "call once" << endl; });
    };

    auto f1 = async(std::launch::async, [&]{fun();});
    auto f2 = async(std::launch::async, [&]{fun();});
    auto f3 = async(std::launch::async, [&]{fun();});

    f1.wait();
    f2.wait();
    f3.wait();

    return 0;
}