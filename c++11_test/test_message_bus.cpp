//
// Created by root on 8/11/21.
//

#include <iostream>
#include <regex>
#include <cstring>

#include "./src/message_bus.h"
#include "./src/singleton.h"

using namespace std;

const string UIScrapCarTopic = "UIScrapCarTopic(void(string))";
const string carbatch1 = "carbatch1(void())";
const string carbatch2 = "carbatch2(void())";
const string carbatch3 = "carbatch3(void())";
const string Af1 = "Af1(void())";

struct ICar {
    ICar(int id, const string &own) : id(id), own(own){}
    ~ICar(){ cout << "~ICar()" << endl; }

    virtual string scrap()=0;

protected:
    int id;
    string own;
};
struct Truck : ICar{
    using ICar::ICar;

    string scrap(){
        this_thread::sleep_for(milliseconds{250});
        cout << "scrap " << own << " truck " << id << endl;
        cout.flush();
        return own;
    }
};
struct Bus : ICar{
    using ICar::ICar;

    string scrap(){
        this_thread::sleep_for(milliseconds{250});
        cout << "scrap " << own << " bus " << id << endl;
        cout.flush();
        return own;
    }
};

struct UI {
    void scrapCarOwnDone(string own){
        cout << "scrap " << own << " car done" << endl;
        cout.flush();
    }
};

struct A : MessageBusModel{
    void f1(){
        cout << "f1()" << endl;
        cout.flush();
    }
};

int main(){
    multimap<string, size_t> msgInfos;
    MessageBus *bus = Singleton<MessageBus>::instance();

    A a;
    a.registerMessage(bus, [&a,bus]{
        a.f1();
        a.removeMessage(bus);
    }, Af1);

    // register message
    {
        vector<string> owns;
        owns.emplace_back("腾讯 行政部门");
        owns.emplace_back("腾讯 技术部门");
        owns.emplace_back("阿里 行政部门");
        owns.emplace_back("阿里 技术部门");
        owns.emplace_back("百度 行政部门");
        owns.emplace_back("百度 技术部门");

        shared_ptr<ICar> car;
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Truck>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch1));
        }
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Bus>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch1));
        }
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Truck>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch2));
        }
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Bus>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch2));
        }
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Truck>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch3));
        }
        for (int i = 0; i < 5; ++i) {
            car = make_shared<Bus>(rand()%90+10, owns[rand()%owns.size()]);
            auto message = [bus, car]{
                bus->asyncSendReq(UIScrapCarTopic, car->scrap());
            };
            msgInfos.emplace(bus->attach(move(message), carbatch3));
        }

        shared_ptr<UI> ui{new UI};
        auto message = [ui, bus](string own){
            ui->scrapCarOwnDone(own);
        };
        msgInfos.emplace(bus->attach(move(message), UIScrapCarTopic));
    }
    // send messge
    {
        bus->asyncSendReq(carbatch2);
    }
    // remove message.不能生成新请求，已经生成的请求会执行完
    {
        vector<pair<string, size_t>> tmp;
        for_each(msgInfos.begin(), msgInfos.end(), [&](typename iterator_traits<decltype(msgInfos.begin())>::reference item){
            regex rg{carbatch2+".*"};
//            if(regex_match(item.first.begin(), item.first.end(), rg)){
//                tmp.emplace_back(item);
//            }
            if(strncmp(carbatch2.c_str(), item.first.c_str(), strlen(carbatch2.c_str()))==0){
                tmp.emplace_back(item);
            }
        });
        for_each(tmp.begin(), tmp.end(), [&](typename iterator_traits<decltype(tmp.begin())>::reference item){
            bus->remove(item.second, item.first);
            msgInfos.erase(item.first);
        });
    }
    // send messge
    {
        bus->asyncSendReq(carbatch2);
    }

    //　模拟其他线程发送请求
    bus->asyncSendReq(Af1);

    return 0;
}

