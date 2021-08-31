//
// Created by root on 7/26/21.
//

#include <iostream>
#include <functional>

using namespace std;

template<typename T>
class Task;

template<typename R, typename ...Args>
class Task<R(Args...)>{
public:
    Task(function<R(Args...)> &&f) : m_fn(move(f)) {}
    Task(function<R(Args...)> &f) : m_fn(f) {}

    R run(Args&&...args){
        return m_fn(forward<Args>(args)...);
    }

    template<typename F>
    auto then(F &&fn) -> Task<typename result_of<F(R)>::type(Args...)>{
        using return_type = typename result_of<F(R)>::type;
        auto func = move(m_fn);
        return Task<return_type(Args...)>{[=](Args&&...args){
            return fn(func(forward<Args>(args)...));
        }};
    }

private:
    function<R(Args...)> m_fn;
};

int f(int i, int j){
    return i*j;
}

int main(){
    Task<int(int, int)> task{[](int i, int j)->int{return i*j;}};
    auto r = task.then([](int i)->int{return i*2;}).then(bind(f, 2, placeholders::_1));
    cout << r.run(1,2) << endl;
    return 0;
}

