//
// Created by root on 8/10/21.
//

#include <iostream>
#include <string>

#include "./src/big_endian_convert.h"

using namespace std;

template<typename T>
void test(){
    T d[2048];
    for (int i = 0; i < sizeof(d) / sizeof(T); ++i) {
        d[i] = rand()*1.0/rand()*100;
    }

    auto str = Big_Endian_Convert<T>::convert_to_string(d, sizeof(d)/sizeof(T));

    T tmp[2048];
    Big_Endian_Convert<T>::convert_from_string(str, tmp, sizeof(d)/sizeof(T));

    for (int i = 0; i < sizeof(d) / sizeof(T); ++i) {
        if(d[i] != tmp[i])
            cout << "error" << endl;
    }
}

int main(){
    test<double>();
    test<int>();
    test<int64_t>();
    test<int16_t>();
    test<float>();

    return 0;
}
