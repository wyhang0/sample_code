//
// Created by root on 8/8/21.
//

#include <iostream>
#include <future>

#include "./src/aspect.h"

using namespace std;

struct AA{
//    void before(string i){
//        cout << "before form AA " << i << endl;
//        cout.flush();
//    }
    void after(string i){
        cout << "after form AA " << i << endl;
        cout.flush();
    }
};
struct BB{
    void before(string i){
        cout << "before form BB " << i << endl;
        cout.flush();
    }
//    void after(string i){
//        cout << "after form BB " << i << endl;
//        cout.flush();
//    }
};
struct CC{
    void before(){
        cout << "before form CC" << endl;
        cout.flush();
    }
//    void after(){
//        cout << "after form CC" << endl;
//        cout.flush();
//    }
};
struct DD{
//    void before(){
//        cout << "before form DD" << endl;
//        cout.flush();
//    }
    void after(){
        cout << "after form DD" << endl;
        cout.flush();
    }
};

void f1(string i){
    cout << "real f1 function " << i << endl;
    cout.flush();
}
void f2(){
    cout << "real f2 function" << endl;
    cout.flush();
}

int main(){
    invoke<AA,BB>(function<void(string)>{f1}, "seg");
    cout << endl;
    function<void(string)> f{[](string s){invoke<AA,BB>(function<void(string)>{f1}, s);}};
    invoke<CC,DD>(function<void()>{[=]{f("eg");}});
    cout << endl;
    invoke<CC,DD>(function<void()>{f2});

    return 0;
}