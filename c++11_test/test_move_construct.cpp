//
// Created by ubuntu on 2021/7/16.
//

#include <iostream>

#include "./src/student.h"

using namespace std;

class Team : Student{};

void testInherit(){
    Team t;
    Team t2{move(t)};
}

int main(){
    string s{"s"};
    int i1=1, i2=2;

    cout << R"(student{"12", 1, 2}:)" << endl;
    Student s1{"12", 1, 2};
    cout << endl;

    cout << R"(student{s, i1, i2}:)" << endl;
    Student s2{s, i1, i2};
    cout << endl;

    cout << R"(student{s, move(i1), move(i2)}:)" << endl;
    Student s3{s, move(i1), move(i2)};
    cout << endl;

    cout << R"(student{s3}:)" << endl;
    Student s4{s3};
    cout << endl;

    cout << R"(student{s3}:)" << endl;
    Student s5{move(s3)};
    cout << endl;

    testInherit();

    return 0;
}

