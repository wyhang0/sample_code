//
// Created by root on 8/10/21.
//

#ifndef C__11_TEST_BIG_ENDIAN_CONVERT_H
#define C__11_TEST_BIG_ENDIAN_CONVERT_H

#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

template<typename T, typename enable_if<is_arithmetic<T>::value>::type* = nullptr>
struct Big_Endian_Convert{
    static string convert_to_string(T *t, size_t size){
        if(m_is_big_endian){
            ostringstream out;
            for (int i = 0; i < size; ++i) {
                out.write((char*)&t[i], sizeof(T));
            }
            return move(out.str());
        }else{
            ostringstream out;
            for (int i = size-1; i >=0; --i) {
                out.write((char*)&t[i], sizeof(T));
            }
            string str = out.str();
            reverse(str.begin(), str.end());
            return move(str);
        }
    }
    static void convert_from_string(string &str, T *t, size_t size){
        if(str.size() < size*sizeof(T))
            return;
        if(m_is_big_endian){
            istringstream in{str};
            for (int i = 0; i < size; ++i) {
                in.read((char*)&t[i], sizeof(T));
            }
        }else{
            reverse(str.begin(), str.end());
            istringstream in{str};
            for (int i = size-1; i >= 0; --i) {
                in.read((char*)&t[i], sizeof(T));
            }
        }
    }
    static void convert_from_string(string &&str, T *t, size_t size){
        convert_from_string(str, t, size);
    }

private:
    static short m_s;
    static bool m_is_big_endian;
};

template<typename T, typename enable_if<is_arithmetic<T>::value>::type* P>
short Big_Endian_Convert<T, P>::m_s = 0x01;
template<typename T, typename enable_if<is_arithmetic<T>::value>::type* P>
bool Big_Endian_Convert<T, P>::m_is_big_endian = *((char*)&Big_Endian_Convert<T>::m_s)!=0x01;

#endif //C__11_TEST_BIG_ENDIAN_CONVERT_H
