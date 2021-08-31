//
// Created by ubuntu on 2021/7/16.
//

#ifndef C__11_TEST_STUDENT_H
#define C__11_TEST_STUDENT_H

#include <cstring>
#include <iostream>

using namespace std;

class Student{
public:
    Student(){
        cout << "student()" << endl;
    }
    Student(string &name, int age, int score):
            name(name),age(age),score(score){
        cout << "student(string &name, int age, int score)" << endl;
    }
    Student(string &&name, int &&age, int &&score):
            name(move(name)),age(age),score(score){
        cout << "student(string &&name, int &&age, int &&score)" << endl;
    }
    Student(const Student &student):
            name(student.name),age(student.age),score(student.score){
        if(student.data != nullptr){
            data = new char[dataSize];
            memcpy(data, student.data, dataSize*sizeof(char));
        }
        cout << "student(const student &student)" << endl;
    }
    Student(Student &&student):
            name(move(student.name)),age(student.age),score(student.score),
            data(student.data),dataSize(student.dataSize){
        student.name = "";
        student.data = nullptr;
        student.dataSize = 0;
        cout << "student(student &&student)" << endl;
    }
    Student& operator=(const Student &student){
        if(this != &student){
            name = student.name;
            age = student.age;
            score = student.score;
            if(student.data != nullptr){
                data = new char[dataSize];
                memcpy(data, student.data, dataSize*sizeof(char));
            }

            cout << "student& operator=(const student &student)" << endl;
        }
        return *this;
    }
    Student& operator=(Student &&student){
        if(this != &student){
            name = move(student.name);
            age = student.age;
            score = student.score;
            data = student.data;
            dataSize = student.dataSize;

            student.name = "";
            student.data = nullptr;
            student.dataSize = 0;

            cout << "student& operator=(student &&student)" << endl;
        }
        return *this;
    }
    ~Student(){
        if(data != nullptr){
            delete[] data;
        }
        cout << "~student()" << endl;
        cout.flush();
    }

    friend ostream& operator<<(ostream &out, const Student &student){
        out <<"{" << student.name << " " << student.age << " " << student.score << "}";
        return out;
    }

public:
    string name;
    int age;
    int score;
    char *data{nullptr};
    int dataSize{0};

};

#endif //C__11_TEST_STUDENT_H
