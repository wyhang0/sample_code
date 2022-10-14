//
// Created by ubuntu on 10/12/22.
//

#ifndef C__11_TEST_SQLCONNECTIONSQLITE_H
#define C__11_TEST_SQLCONNECTIONSQLITE_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/pfr.hpp>

#include "../src/sqlite_cpp.h"

class SqlConnectionSqlite {
public:
    SqlConnectionSqlite(string dbPath);
    ~SqlConnectionSqlite();

    bool begin();

    bool rollBack();

    bool commit();

    bool execute(string sql);

    int getLastErrorCode();

    string getLastError();

    bool queryCountSql(int &count, string sql);

    template<class Struct>
    bool querySql(std::vector<boost::shared_ptr<Struct>> &vecObjs, string sql){
        cout << sql << endl;
        do{
            if(!db.query(sql)){
                break;
            }
            while(db.next()){
                auto st = boost::make_shared<Struct>();
                SqlQueryBindValues::sqlQueryBindValues(db, st);
                vecObjs.push_back(st);
            }
            return true;
        }while(false);

        return false;
    }

private:
    template<int...>
    struct IndexSeq{};

    template<int N, int...Indexes>
    struct MakeIndexes{
        using type = typename MakeIndexes<N - 1, N - 1, Indexes...>::type;
    };

    template<int...Indexes>
    struct MakeIndexes<0, Indexes...>{
        using type = IndexSeq<Indexes...>;
    };

    class SqlQueryBindValues{
    public:
        template<typename Struct>
        static void sqlQueryBindValues(SqliteCpp &db, boost::shared_ptr<Struct> st){
            using Index = typename MakeIndexes<boost::pfr::tuple_size_v<Struct>>::type;
            __sqlQueryBindValues(Index{}, db, st);
        }

    private:
        template<int ...Indexes, typename Struct>
        static void __sqlQueryBindValues(IndexSeq<Indexes...> &&, SqliteCpp &db, boost::shared_ptr<Struct> st){
            auto func = [](auto &&key, auto &&value){
                key = value;
            };
            std::initializer_list<int>{(func(boost::pfr::get<Indexes>(*st), db.get<boost::pfr::tuple_element_t<Indexes, Struct>>(Indexes)),0)...};
        }
    };

private:
    SqliteCpp db;
};


#endif //C__11_TEST_SQLCONNECTIONSQLITE_H
