//
// Created by root on 7/27/21.
//

#include <iostream>
#include <typeinfo>

#include "./src/<template....T>.h"
#include "./src/student.h"

using namespace std;

int main(){
    int i = 1;
    print1();
    print1(i,2,3,"eg");
    cout << "---------------------------------" << endl;

    print2();
    print2(i,2,3,"eg");
    cout << "---------------------------------" << endl;

    cout << SumSizeof<int, int>::value <<endl;
    cout << "---------------------------------" << endl;

    cout << ContainerType<double>::value << endl;
    cout << ContainerType<double, int, double, char>::value << endl;
    cout << ContainerType<double, int, float, char>::value << endl;

    cout << TypeIndex<double>::value << endl;
    cout << TypeIndex<double, int, double, char>::value << endl;
    cout << TypeIndex<double, int, float, char>::value << endl;
    
    cout << typeid(IndexType<0, double>::type).name() << endl;
    cout << typeid(IndexType<2, double, int, char, float>::type).name() << endl;
    cout << typeid(IndexType<3, double, int, char, float>::type).name() << endl;
#if __cplusplus > 201103L
    Members<int, string, int> members{1,"eg", 2};
    members.print();
    members.set<0>(2);
    members.set<1>(string("22"));
    members.set<2>(3);
    members.print();
    const int j = 1;
    auto &i1 = members.get<0>();
    auto &s1 = members.get<j>();
    auto &i2 = members.get<2>();
    i1 = 1;
    s1 = "eg";
    i2 = 2;
    members.print();
    Members<int, string, int, Student> member2{1,"eg", 2, {"wy", 18, 100}};
    member2.print();
    cout << "---------------------------------" << endl;
#endif
    tuple<int, int, int, string> t = make_tuple(1,2,3,"eg");
    print_tuple(t);
    print_tuple(2, t);
    print_tuple(3, t);
    cout << "---------------------------------" << endl;

    return 0;
}