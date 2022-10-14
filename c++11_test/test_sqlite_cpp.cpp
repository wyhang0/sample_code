//
// Created by root on 8/15/21.
//

#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <thread>

#include "./src/sqlite_cpp.h"
#include "./src/big_endian_convert.h"

#include <chrono>
#include <mutex>
#include <shared_mutex>

#include <string>
#include "./sqlModel/SqlConnectionSqlite.h"
#include "./sqlModel/SqlStatementConstructor.h"
#include "./sqlModel/SqlConnectionPoolSharedMutex.h"

static std::shared_mutex mt;

using namespace std;
using namespace chrono;

struct School{
    int id;
    string name;
    string addr;
    int phone;
};

struct Student{
    int id;
    string name;
    string addr;
    int phone;
    int schoolId;
};

int main(){
    {
        do{
            srand(time(nullptr));

            auto sqlConnectionPool = boost::make_shared<SqlConnectionPoolSharedMutex<SqlConnectionSqlite> >(5, "School.db");
            //create table
            {
                auto execute = sqlConnectionPool->getExecuteConnection();
                if(execute){
                    SqlStatementConstructor sqlStatementConstructor;
                    string sql = R"(CREATE TABLE if not exists "School" (
                            "id"	INTEGER NOT NULL,
                            "name"	TEXT NOT NULL,
                            "addr"	INTEGER NOT NULL,
                            "phone"	INTEGER NOT NULL,
                            PRIMARY KEY("id")
                        ))";
                    if (!execute->execute(sql)) {
                        cout << "err: " << execute->getLastError() << endl;
                        break;
                    }

                    sql = R"(CREATE TABLE if not exists "Student" (
                        "id"	INTEGER NOT NULL,
                        "name"	TEXT NOT NULL,
                        "addr"	TEXT NOT NULL,
                        "phone"	INTEGER NOT NULL DEFAULT 1,
                        "schoolId"	INTEGER NOT NULL,
                        CONSTRAINT "schoolId" FOREIGN KEY("schoolId") REFERENCES "School"("id") ON DELETE CASCADE ON UPDATE NO ACTION,
                        UNIQUE("name" ASC),
                        UNIQUE("id" ASC),
                        PRIMARY KEY("id" AUTOINCREMENT)
                    ))";
                    if (!execute->execute(sql)) {
                        cout << "err: " << execute->getLastError() << endl;
                        break;
                    }
                }
            }

            {
                auto executeFunc = [sqlConnectionPool](){
                    SqlStatementConstructor sqlStatementConstructor;
                    for (int i = 0; i < 500; ++i) {
                        auto execute = sqlConnectionPool->getExecuteConnection();
                        if(!execute)
                            continue;
                        cout << this_thread::get_id() << " exec" << endl;
                        execute->begin();
                        for (int i = 0, j = 1; i < 5; ++i) {
                            School school;
                            school.id = random();
                            school.name = to_string(j++);
                            school.addr = j++;
                            school.phone = j++;

                            Student student;
                            student.id = random();
                            student.name = to_string(j++);
                            student.addr = to_string(j++);
                            student.phone = j++;
                            student.schoolId = school.id;

                            if(!execute->execute(sqlStatementConstructor.insertOrReplaceSqlStr(school))){
                                cout << execute->getLastError() << endl;
                                execute->rollBack();
                                break;
                            }
                            if(!execute->execute(sqlStatementConstructor.insertOrReplaceSqlStr(student))){
                                cout << execute->getLastError() << endl;
                                execute->rollBack();
                                break;
                            }
                        }
                        execute->commit();
                    }
                };
                auto queryFunc = [sqlConnectionPool](){
                    SqlStatementConstructor sqlStatementConstructor;
                    for (int i = 0; i < 500; ++i) {
                        vector<boost::shared_ptr<Student>> studentV;
                        vector<boost::shared_ptr<School>> schoolV;

                        auto query = sqlConnectionPool->getQueryConnection();
                        if(!query)
                            continue;
                        cout << this_thread::get_id() << " query" << endl;
                        if(!query->querySql(schoolV, sqlStatementConstructor.selectSqlStr<School>(sqlStatementConstructor.sqlQueryAndConditions("like", "name", "%12%")))){
                            cout << "err: " << query->getLastError() << endl;
                            break;
                        }
                        if(!query->querySql(studentV, sqlStatementConstructor.selectSqlStr<Student>(sqlStatementConstructor.sqlQueryAndConditions("like", "id", "%12")))){
                            cout << "err: " << query->getLastError() << endl;
                            break;
                        }
                    }
                };
                std::vector<std::thread> ths;
                for (int i = 0; i < 10; ++i) {
                    if(i%2 == 0){
                        ths.push_back(std::thread(executeFunc));
                    }else{
                        ths.push_back(std::thread(queryFunc));
                    }
                }
                for (auto &th : ths){
                    th.join();
                }
            }
            cout << endl;
        }while(false);

    }
    {
        do{
            SqlConnectionSqlite conn("School.db");
            SqlStatementConstructor sqlStatementConstructor;
            string sql = R"(CREATE TABLE if not exists "School" (
                            "id"	INTEGER NOT NULL,
                            "name"	TEXT NOT NULL,
                            "addr"	INTEGER NOT NULL,
                            "phone"	INTEGER NOT NULL,
                            PRIMARY KEY("id")
                        ))";
            if(!conn.execute(sql)){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }

            sql = R"(CREATE TABLE if not exists "Student" (
                        "id"	INTEGER NOT NULL,
                        "name"	TEXT NOT NULL,
                        "addr"	TEXT NOT NULL,
                        "phone"	INTEGER NOT NULL DEFAULT 1,
                        "schoolId"	INTEGER NOT NULL,
                        CONSTRAINT "schoolId" FOREIGN KEY("schoolId") REFERENCES "School"("id") ON DELETE CASCADE ON UPDATE NO ACTION,
                        UNIQUE("name" ASC),
                        UNIQUE("id" ASC),
                        PRIMARY KEY("id" AUTOINCREMENT)
                    ))";
            if(!conn.execute(sql)){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }

            conn.begin();
            for (int i = 0, j = 1; i < 60000; ++i) {
                School school;
                school.id = j++;
                school.name = to_string(j++);
                school.addr = to_string(j++);
                school.phone = j++;

                Student student;
                student.id = j++;
                student.name = to_string(j++);
                student.addr = to_string(j++);
                student.phone = j++;
                student.schoolId = school.id;

                if(!conn.execute(sqlStatementConstructor.insertOrReplaceSqlStr(school))){
                    cout << conn.getLastError() << endl;
                    conn.rollBack();
                    break;
                }
                if(!conn.execute(sqlStatementConstructor.insertOrReplaceSqlStr(student))){
                    cout << conn.getLastError() << endl;
                    conn.rollBack();
                    break;
                }
            }
            conn.commit();

            if(!conn.execute(sqlStatementConstructor.deleteSqlStr<School>(sqlStatementConstructor.sqlQueryAndConditions("=", "id", 1, "name", "2")))){
                cout << "err: " << conn.getLastError() << conn.getLastErrorCode() << endl;
                break;
            }

            if(!conn.execute(sqlStatementConstructor.deleteSqlStr<School>(sqlStatementConstructor.sqlQueryOrConditions("=", "id", 1, "id", "22")))){
                cout << "err: " << conn.getLastError() << conn.getLastErrorCode() << endl;
                break;
            }

            vector<boost::shared_ptr<Student>> studentV;
            vector<boost::shared_ptr<School>> schoolV;
            if(!conn.querySql(studentV, sqlStatementConstructor.selectSqlStr<Student>(""))){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }
            if(!conn.querySql(schoolV, sqlStatementConstructor.selectSqlStr<School>(sqlStatementConstructor.sqlQueryAndConditions("like", "name", "%12%")))){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }
            if(!conn.querySql(schoolV, sqlStatementConstructor.selectSqlStr<School>(""))){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }

            int count;
            if(!conn.queryCountSql(count, sqlStatementConstructor.selectCountSqlStr<School>(sqlStatementConstructor.sqlQueryAndConditions(">", "id", 1)))){
                cout << "err: " << conn.getLastError() << endl;
                break;
            }
            cout << count << endl;
        }while(false);

        return 0;
    }
    {
//        sqlite设置多线程模式使用连接池，一个连接只能同时被一个线程使用。
//        读写数据库操作互斥，同一时间要么进行写操作，要么进行读操作。
//        可以并发读，不能并发写。
        std::vector<std::thread> th;
        for (int i = 0; i < 4; i++) {
            th.push_back(std::thread([i]() {
                do {
                    time_point<system_clock> now{system_clock::now()};
                    string str{"wseg"};
                    double d[10];
                    srand(system_clock::now().time_since_epoch().count());
                    for (int j = 0; j < sizeof(d) / sizeof(double); ++j) {
                        d[i] = rand() * 1.0 / rand() * 100;
                    }

                    auto db = std::make_shared<SqliteCpp>("test.db");
                    {
                        {
                            // 创建表
                            string sql = R"(CREATE TABLE if not exists `t1` (
                            `key`	TEXT,
                            `value`	INTEGER,
                            `data`	blob,
                            PRIMARY KEY(key,value)
                        );)";
                            std::lock_guard<std::shared_mutex> locker(mt);
                            if (!db->execute(sql)) {
                                cout << i << "\terr: " << db->getLastError() << endl;
                                break;
                            }
                        }
                        {
                            auto spectrum = Big_Endian_Convert<double>::convert_to_string(d,
                                                                                          sizeof(d) / sizeof(double));
                            string sql = "insert into t1 values(?,?,?);";
                            // 插入数据
                            //　开启事务,提高插入速度
                            std::lock_guard<std::shared_mutex> locker(mt);
                            db->begin();
                            if (db->prepare(sql)) {
                                for (int j = i * 100000; j < (i + 1) * 100000; ++j) {
                                    if (!db->executeArgs("key", j, blob{spectrum.data(), spectrum.size()})) {
                                        cout << i << "\terr: " << db->getLastError() << endl;
                                        db->rollBack();
                                        break;
                                    }
                                }
                            }
                            db->commit();
                            cout << duration_cast<milliseconds>(system_clock::now() - now).count() << endl;
                        }
                    }

                    {
                        //　查询数据
                        string sql = "select * from t1;";
                        std::shared_lock<std::shared_mutex> locker(mt);
                        if (!db->query(sql))
                            break;
                        while (db->next()) {
                            cout << i << '\t' << db->get<string>(0) << " " << db->get<int>(1) << endl;
                            string blob_str = db->get<blob>(2).toString();

                            memset(d, 0, sizeof d);
                            Big_Endian_Convert<double>::convert_from_string(blob_str, d, sizeof(d) / sizeof(double));
                            for (int i = 0; i < sizeof(d) / sizeof(double); i++) {
                                cout << d[i] << '\t';
                            }
                            cout << endl;
                        }
                    }
                } while (false);
            }));
        }

        for (int i = 0; i < th.size(); i++) {
            th[i].join();
        }
    }
    return 0;
}
