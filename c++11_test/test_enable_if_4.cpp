//
// Created by root on 7/15/21.
//

#include "iostream"

using namespace std;

#if 0
template<typename T>
typename enable_if<is_pointer<T>::m_value>::type fun(T t){
    cout << "point" << endl;
}

template<typename T>
typename enable_if<is_integral<T>::m_value>::type fun(T t){
    cout << "int" << endl;
}

template<typename T>
typename enable_if<is_pointer<T>::m_value>::type fun(){
    cout << "point" << endl;
}

template<typename T>
typename enable_if<is_integral<T>::m_value>::type fun(){
    cout << "int" << endl;
}
#else
template<typename T, typename enable_if<is_pointer<T>::value, T>::type* = nullptr>
void fun(T t){
    cout << "point" << endl;
}

template<typename T, typename enable_if<is_integral<T>::value, T>::type* = nullptr>
void fun(T t){
    cout << "int" << endl;
}

template<typename T, typename enable_if<is_pointer<T>::value, T>::type* = nullptr>
void fun(){
    cout << "point" << endl;
}

template<typename T, typename enable_if<is_integral<T>::value, T>::type* = nullptr>
void fun(){
    cout << "int" << endl;
}
#endif

int main(){
    fun((void*)0);
    fun(1);

    fun<void*>();
    fun<int>();

    string str;
    str.size();

    return 0;
}