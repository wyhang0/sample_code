//
// Created by ubuntu on 10/13/22.
//

#ifndef C__11_TEST_SQLSTATEMENTCONSTRUCTOR_H
#define C__11_TEST_SQLSTATEMENTCONSTRUCTOR_H

#include <cxxabi.h>
#include <sstream>

using namespace std;

class SqlStatementConstructor{
public:
    template<class Struct>
    string insertAutoIncrementIdSqlPrepareStr(){
        auto columns = typename std::remove_reference<Struct>::type().getColumns();
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert into " << tableName << " (";
        for(size_t i=1; i<columns.size()-1; i++){
            ss << columns[i] << ",";
        }
        ss << columns[columns.size()-1] << ") values (";
        for(size_t i=1; i<boost::pfr::tuple_size_v<Struct>-1; i++){
            ss << "?,";
        }
        ss << "?)";

        sql = ss.str();

        return sql;
    }

    template<class Struct>
    string insertAutoIncrementIdSqlStr(Struct &&st){
        auto columns = typename std::remove_reference<Struct>::type().getColumns();
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert into " << tableName << " (";
        for(size_t i=1; i<columns.size()-1; i++){
            ss << columns[i] << ",";
        }
        ss << columns[columns.size()-1] << ") values (";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st), 1);

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertSqlPrepareStr(){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert into " << tableName << " values(";
        for(size_t i=0; i<boost::pfr::tuple_size_v<Struct>-1; i++){
            ss << "?,";
        }
        ss << "?)";

        sql = ss.str();

        return sql;
    }

    template<class Struct>
    string insertSqlStr(Struct &&st){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert into " << tableName << " values(";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st), 0);

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertSql(string &str){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert into " << tableName << " values " << str;

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertOrIgnoreSqlStr(Struct &&st){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert or ignore into " << tableName << " values(";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st), 0);

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertOrReplaceSqlStr(Struct &&st){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert or replace into " << tableName << " values(";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st), 0);

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertOrReplaceSqlPrepareStr(){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "insert or replace into " << tableName << " values(";
        for(size_t i=0; i<boost::pfr::tuple_size_v<Struct>-1; i++){
            ss << "?,";
        }
        ss << "?)";

        sql = ss.str();

        return sql;
    }

    template<class Struct>
    string updateAutoIncrementIdSqlStr(Struct &&st){
        auto columns = typename std::remove_reference<Struct>::type().getColumns();
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "update " << tableName << " SET ";
        SqlUpdateBindValues::sqlUpdateBindValues(ss, std::forward<Struct>(st), 1, columns);

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string updateSqlStr(Struct &&st, int startIndex){
        auto columns = typename std::remove_reference<Struct>::type().getColumns();
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "update " << tableName << " SET ";
        SqlUpdateBindValues::sqlUpdateBindValues(ss, std::forward<Struct>(st), startIndex, columns);

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string deleteSqlStr(vector<pair<vector<string>, string>> conditions){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "delete from " << tableName << " ";

        string conditionStr;
        for(auto &condition : conditions){
            if(conditionStr.empty()){
                conditionStr += condition.first[0] + " " + condition.second + " ";
            }else{
                conditionStr += condition.first[1] + " " + condition.second + " ";
            }
        }
        ss << conditionStr;

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string deleteSqlStr(string condition){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "delete from " << tableName << " where " << condition;

        string conditionStr;

        ss << conditionStr;

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string selectSqlStr(vector<pair<vector<string>, string>> conditions){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "select * from " << tableName << " ";

        string conditionStr;
        for(auto &condition : conditions){
            if(conditionStr.empty()){
                conditionStr += condition.first[0] + " " + condition.second + " ";
            }else{
                conditionStr += condition.first[1] + " " + condition.second + " ";
            }
        }
        ss << conditionStr;

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string selectCountSqlStr(vector<pair<vector<string>, string>> conditions){
        string sql;
        string tableName;

        int status;
        std::unique_ptr<char, void (*)(void *)> res{
                abi::__cxa_demangle(typeid(Struct).name(), nullptr, nullptr, &status), std::free};
        if (status == 0)
            tableName = res.get();

        ss.str("");
        ss << "select count(1) from " << tableName << " ";

        string conditionStr;
        for(auto &condition : conditions){
            if(conditionStr.empty()){
                conditionStr += condition.first[0] + " " + condition.second + " ";
            }else{
                conditionStr += condition.first[1] + " " + condition.second + " ";
            }
        }
        ss << conditionStr;

        sql = ss.str();
        ss.str("");

        return sql;
    }

    /**
     *
     * @param joinStr
     * '>' '<' '=' 'like'
     * @param args
     * key1, key1Value, key2, key2Value
     * @return
     * joinStr = "="
     * {{"", ""}, "key1 = key1 and key2 = key2Value"}
     */
    template<typename ...Args>
    pair<vector<string>, string> sqlQueryAndConditions(string joinStr, Args &&...args){
        return {{"where", "and"} ,SqlQueryAndConditions::sqlQueryAndConditions(joinStr, std::forward<Args>(args)...)};
    }

    template<typename ...Args>
    pair<vector<string>, string> sqlQueryOrConditions(string joinStr, Args &&...args){
        return {{"where", "or"}, SqlQueryOrConditions::sqlQueryOrConditions(joinStr, std::forward<Args>(args)...)};
    }

    pair<vector<string>, string> sqlQueryLimitConditions(int limit, int offset = 0){
        stringstream ss;
        ss << "limit " << limit << " offset " << offset;
        return {{"", ""}, ss.str()};
    }

    template<typename ...Args>
    pair<vector<string>, string> sqlQueryOrderByConditions(Args &&...args){
        stringstream ss;
        string sql;
        ss << "order by ";

        auto init = [](stringstream &ss, string key){
            ss << key << ',';
        };
        std::initializer_list<int>{(init(ss, args),0)...};

        sql = ss.str();
        sql = sql.substr(0, sql.size()-1);

        return {{"", ""}, sql};
    }

    template<typename ...Args>
    pair<vector<string>, string> sqlQueryGroupByConditions(Args &&...args){
        stringstream ss;
        string sql;
        ss << "group by ";

        auto init = [](stringstream &ss, string key){
            ss << key << ',';
        };
        std::initializer_list<int>{(init(ss, args),0)...};

        sql = ss.str();
        sql = sql.substr(0, sql.size()-1);
        return {{"", ""}, sql};
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

    class SqlInsertBindValues{
    public:
        template<typename Struct>
        static void sqlInsertBindValues(stringstream &ss, Struct &&st, int startIndex){
            using Index = typename MakeIndexes<boost::pfr::tuple_size<typename std::remove_reference<Struct>::type>::value>::type;
            __sqlInsertBindValues(Index{}, ss, forward<decltype(st)>(st), startIndex);
        }

    private:
        template<typename T>
        static void init(stringstream &ss, T &&t, int index, int startIndex){
            if(index >= startIndex)
                ss << t << ',';
        }
        static void init(stringstream &ss, string str, int index, int startIndex){
            if(index >= startIndex)
                ss << "'" << str << "'" << ',';
        }

        template<int ...Indexes, typename Struct>
        static void __sqlInsertBindValues(IndexSeq<Indexes...> &&, stringstream &ss,  Struct &&st, int startIndex){
            std::initializer_list<int>{(init(ss, boost::pfr::get<Indexes>(st), Indexes, startIndex),0)...};
        }
    };

    class SqlUpdateBindValues{
    public:
        template<typename Struct>
        static void sqlUpdateBindValues(stringstream &ss, Struct &&st, int startIndex, vector<string> &columns){
            using Index = typename MakeIndexes<boost::pfr::tuple_size<typename std::remove_reference<Struct>::type>::value>::type;
            __sqlUpdateBindValues(Index{}, ss, forward<decltype(st)>(st), startIndex, columns);
        }

    private:
        template<typename T>
        static void init(stringstream &ss, T &&t, int index, int startIndex, vector<string> &columns){
            if(index >= startIndex)
                ss << columns[index] << "=" << t << ',';
        }
        static void init(stringstream &ss, string str, int index, int startIndex, vector<string> &columns){
            if(index >= startIndex)
                ss << columns[index] << "='" << str << "'" << ',';
        }
        template<typename T>
        static void where(stringstream &ss, T &&t, int index, int startIndex, vector<string> &columns){
            if(index < startIndex)
                ss << columns[index] << "=" << t << ',';
        }
        static void where(stringstream &ss, string str, int index, int startIndex, vector<string> &columns){
            if(index < startIndex)
                ss << columns[index] << "='" << str << "'" << ',';
        }

        template<int ...Indexes, typename Struct>
        static void __sqlUpdateBindValues(IndexSeq<Indexes...> &&, stringstream &ss,  Struct &&st, int startIndex, vector<string> &columns){
            std::initializer_list<int>{(init(ss, boost::pfr::get<Indexes>(st), Indexes, startIndex, columns),0)...};

            string sql = ss.str();
            ss.str("");
            sql = sql.substr(0, sql.size()-1);
            ss << sql << " where ";

            std::initializer_list<int>{(where(ss, boost::pfr::get<Indexes>(st), Indexes, startIndex, columns),0)...};

            sql = ss.str();
            ss.str("");
            sql = sql.substr(0, sql.size()-1);
            ss << sql;
        }
    };

    class SqlQueryAndConditions{
    public:
        /**
         *
         * @tparam Args
         * @param joinStr
         * '>' '<' '=' 'like'
         * @param args
         * key1, key1Value, key2, key2Value
         * @return
         * joinStr = "="
         * key1 = key1 and key2 = key2Value
         */
        template<typename ...Args>
        static string sqlQueryAndConditions(string joinStr, Args &&...args){
            stringstream ss;
            __sqlQueryAndConditions(ss, joinStr, forward<Args>(args)...);
            return ss.str();
        }

    private:
        static void __sqlQueryAndConditions(stringstream &ss, string joinStr){
            string tmp = ss.str();
            ss.str("");
            ss << tmp.substr(0, tmp.size() - 5); //最后去掉" and "
        }

        template<typename First, typename ...Rest>
        static void __sqlQueryAndConditions(stringstream &ss, string joinStr, First &&key, Rest&&...rest){
            ss << key << " " << joinStr << " ";
            __sqlQueryAndConditionsValue(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename First, typename ...Rest>
        static void __sqlQueryAndConditionsValue(stringstream &ss, string joinStr, First &&value, Rest&&...rest){
            ss << value << " and ";
            __sqlQueryAndConditions(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename ...Rest>
        static void __sqlQueryAndConditionsValue(stringstream &ss, string joinStr, string value, Rest&&...rest){
            ss << "'" << value << "'" << " and ";
            __sqlQueryAndConditions(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename ...Rest>
        static void __sqlQueryAndConditionsValue(stringstream &ss, string joinStr, const char *value, Rest&&...rest){
            ss << "'" << value << "'" << " and ";
            __sqlQueryAndConditions(ss, joinStr, forward<Rest>(rest)...);
        }
    };

    class SqlQueryOrConditions{
    public:
        template<typename ...Args>
        static string sqlQueryOrConditions(string joinStr, Args &&...args){
            stringstream ss;
            __sqlQueryOrConditions(ss, joinStr, forward<Args>(args)...);
            return ss.str();
        }

    private:
        static void __sqlQueryOrConditions(stringstream &ss, string joinStr){
            string tmp = ss.str();
            ss.str("");
            ss << tmp.substr(0, tmp.size() - 4); //最后去掉" or "
        }

        template<typename First, typename ...Rest>
        static void __sqlQueryOrConditions(stringstream &ss, string joinStr,  First &&key, Rest&&...rest){
            ss << key << " " << joinStr << " ";
            __sqlQueryOrConditionsValue(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename First, typename ...Rest>
        static void __sqlQueryOrConditionsValue(stringstream &ss, string joinStr,  First &&value, Rest&&...rest){
            ss << value << " or ";
            __sqlQueryOrConditions(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename ...Rest>
        static void __sqlQueryOrConditionsValue(stringstream &ss, string joinStr,  string value, Rest&&...rest){
            ss << "'" << value << "'" << " or ";
            __sqlQueryOrConditions(ss, joinStr, forward<Rest>(rest)...);
        }

        template<typename ...Rest>
        static void __sqlQueryOrConditionsValue(stringstream &ss, string joinStr,  const char *value, Rest&&...rest){
            ss << "'" << value << "'" << " or ";
            __sqlQueryOrConditions(ss, joinStr, forward<Rest>(rest)...);
        }
    };

private:
    std::stringstream ss;
};

#endif //C__11_TEST_SQLSTATEMENTCONSTRUCTOR_H
