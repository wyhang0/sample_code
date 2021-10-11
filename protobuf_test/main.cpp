//
// Created by Lenovo on 2021/9/24.
//

#include <iostream>
#include <mutex>
#include <queue>
#include <forward_list>
#include <algorithm>

#include "a.pb.h"

using namespace std;
using namespace Test;

int main(){
    priority_queue<int> a;
    once_flag oc;
    call_once(oc, []{ cout << "hello world!" << endl; });

    PathFreq pathFreq;
    MsgPathUnit msgPathUnit;
    Path path;
    MapPath mapPath;
    Paths paths;
    MapConfig mapConfig;
    SpecConfig specConfig;
    specConfig.set_s("w3g");
    cout << specConfig.s() << endl;
    *specConfig.mutable_s() = "111111111111111111111111111111111";
    cout << specConfig.s() << endl;


    return 0;
}
