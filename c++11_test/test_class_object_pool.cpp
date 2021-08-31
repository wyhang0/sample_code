//
// Created by root on 8/5/21.
//
#include <iostream>
#include <array>

#include "./src/class_object_pool.h"
#include "./src/student.h"
#include "./src/singleton.h"

using namespace std;

void f(int i){
    using Type = double[5];
    typedef double _Type[i];
    using TT = typename remove_pointer<typename remove_all_extents<Type>::type>::type;
    _Type a;
    Type b;
    a[0] = 10;
    b[0] = 32;
    size_t s = sizeof (Type)/ sizeof (TT);
}

int main(){
    f(13);
    ClassObjectPool<Student> *spool = Singleton<ClassObjectPool<Student>>::instance();
    {
        spool->init(1);
        {
            auto s = spool->get<>();
            s.operator bool();
        }
        {
            auto s = spool->get<>();
            s.operator bool();
        }
    }
    {
        spool->init(1, "seg", 18, 100);
        spool->init(1, "s", 18, 100);
        {
            auto s1 = spool->get<const char* const, int, int>();
            s1.operator bool();
            auto s2 = spool->get<const char* const&&, int, int>();
            s2.operator bool();
            auto s3 = spool->get<const char*, int, int>();
            s3.operator bool();
        }
        {
            auto s1 = spool->get<const char* const, int, int>();
            s1.operator bool();
            auto s2 = spool->get<const char*, int, int>();
            s2.operator bool();
            auto s3 = spool->get<const char*, int, int>();
            s3.operator bool();
        }
    }
    {
        string str{"wy"};
        spool->init(1, str, 18, 100);
        str = "ljn";
        spool->init(1, str, 18, 100);
        {
            auto s1 = spool->get<string&&, int, int>();
            s1.operator bool();
            auto s2 = spool->get<volatile const string&, int, int>();
            s2.operator bool();
            auto s3 = spool->get<string, int, int>();
            s3.operator bool();
        }
        {
            auto s1 = spool->get<string&&, int, int>();
            s1.operator bool();
            auto s2 = spool->get<string, int, int>();
            s2.operator bool();
            auto s3 = spool->get<string, int, int>();
            s3.operator bool();
        }
    }

    {
        ClassObjectPool<array<double, 10>> *dpool = Singleton<ClassObjectPool<array<double, 10>>>::instance();
        dpool->init(1);
        auto a = dpool->get<>();
        for (int i = 0; i < a->size(); ++i) {
            a->begin()[i] = i+1;
        }
        for (int i = 0; i < a->size(); ++i) {
            cout << a->begin()[i] << endl;
        }
    }


    return 0;
}