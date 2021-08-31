//
// Created by ubuntu on 2021/8/21.
//

#include <iostream>

#include "./src/variant.h"

using namespace std;

int main(){
    using V = Variant<int, char, double>;

    int x = 10;
    V v{x};
    v = 1;
    v = 1.123;
    cout << v.get<double>() << endl;
    cout << v.is<int>() << endl;

    return 0;
}
