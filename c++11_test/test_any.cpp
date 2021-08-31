//
// Created by root on 7/29/21.
//

#include <vector>
#include <tuple>

#include "./src/any.h"
#include "./src/student.h"

using namespace std;

int main(){
    vector<Any> values;

    const int i = 10000;
    float f = 13.14;
    const int *const p = &i;

    values.emplace_back(i);
    values.emplace_back(f);
    values.emplace_back(p);

    tuple<decay<decltype(i)>::type, decay<decltype(f)>::type, decay<decltype(p)>::type> types;

    for (auto item:values) {
        item.visit([](int&i){ cout << i << endl; },
                   [](float &i){ cout << i << endl; },
                   [](const int* &i){ cout << *i << endl; });
    }

    cout << values[0].any_cast<tuple_element<0, decltype(types)>::type>() << endl;
    cout << values[1].any_cast<tuple_element<1, decltype(types)>::type>() << endl;
    cout << values[2].any_cast<tuple_element<2, decltype(types)>::type>() << endl;

    auto &v1 = values[0].any_cast<tuple_element<0, decltype(types)>::type>();
    auto &v2 = values[1].any_cast<tuple_element<1, decltype(types)>::type>();
    auto &v3 = values[2].any_cast<tuple_element<2, decltype(types)>::type>();

    cout << v1 << v2 << v3 << endl;

    Any any = i;
    cout << any.any_cast<tuple_element<0, decltype(types)>::type>() << endl;

    Any str = "seg";
    cout << str.any_cast<const char*>() << endl;

    int a[4] = {1,2,3,4};
    Any array = a;
    cout << array.any_cast<int*>() << a << endl;
    int *ap = array.any_cast<int*>();
    cout << ap[0] << ap[1] << ap[2] << ap[3] << endl;

    return 0;
}

