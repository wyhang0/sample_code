//
// Created by root on 8/2/21.
//

#include <iostream>
#include <string>
#include <codecvt>
#include <locale>

using namespace std;

class codecvt_gbk : public codecvt_byname<wchar_t, char, mbstate_t>{
public:
    codecvt_gbk() : codecvt_byname<wchar_t, char, mbstate_t>("zh_CN.GBK"){}
};

int main(){
    string str = "上班了！";
    cout << str << endl;

    wstring_convert<codecvt_utf8<wchar_t>> conv;
    wstring wstring1 = conv.from_bytes(str);
    cout << conv.to_bytes(wstring1) << endl;

    wstring_convert<codecvt_gbk> c;
    wstring1 = c.from_bytes(str);
    cout << c.to_bytes(wstring1) << endl;

    return 0;
}
