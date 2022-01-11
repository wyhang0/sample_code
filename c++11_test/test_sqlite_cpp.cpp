//
// Created by root on 8/15/21.
//

#include <iostream>
#include <cstring>

#include "./src/sqlite_cpp.h"
#include "./src/big_endian_convert.h"

#include <chrono>

using namespace std;
using namespace chrono;

int main(){
    do{
        SqliteCpp db("test.db");
        // 创建表
        string sql = R"(CREATE TABLE if not exists `t1` (
            `key`	TEXT,
            `value`	INTEGER,
            `data`	blob,
            PRIMARY KEY(key,value)
        );)";
        if(!db.execute(sql))
            break;
        // 插入数据
        time_point<system_clock> now{system_clock::now()};
        sql = "insert into t1 values(?,?,?);";
        string str{"wseg"};
        double d[10];
        srand(system_clock::now().time_since_epoch().count());
        for (int i = 0; i < sizeof(d) / sizeof(double); ++i) {
            d[i] = rand()*1.0/rand()*100;
        }
        for(int i=0; i<sizeof(d) / sizeof(double); i++){
            cout << d[i] << '\t';
        }
        cout << endl;
        
        auto spectrum = Big_Endian_Convert<double>::convert_to_string(d, sizeof(d)/sizeof(double));
        cout << strlen(spectrum.data()) << endl;
        //　开启事务,提高插入速度
        db.begin();
        if(db.prepare(sql)){
            for (int i = 0; i < 100; ++i) {
                if(!db.executeArgs("key", i, blob{spectrum.data(), spectrum.size()}) && db.getLastErrorCode()!=SQLITE_CONSTRAINT){
                    db.rollBack();
                    break;
                }
            }
        }
        db.commit();
        cout << duration_cast<milliseconds>(system_clock::now()-now).count() << endl;
        //　查询数据
        sql = "select * from t1;";
        if(!db.query(sql))
            break;
        while (db.next()){
            cout << db.get<string>(0) << " " << db.get<int>(1) << endl;
            string blob_str = db.get<blob>(2).toString();

            memset(d, 0, sizeof d);
            Big_Endian_Convert<double>::convert_from_string(blob_str, d, sizeof(d)/sizeof(double));
            for(int i=0; i<sizeof(d) / sizeof(double); i++){
                cout << d[i] << '\t';
            }
            cout << endl;
        }
    } while (false);
    return 0;
}
