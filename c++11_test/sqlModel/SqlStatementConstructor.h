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
    string insertSqlStr(Struct &&st){
        string sql;
        int status;
        ss.str("");
        ss << "insert into " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status) << " values(";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st));

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string insertOrReplaceSqlStr(Struct &&st){
        string sql;
        int status;
        ss.str("");
        ss << "insert or replace into " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status) << " values(";
        SqlInsertBindValues::sqlInsertBindValues(ss, std::forward<Struct>(st));

        sql = ss.str();
        ss.str("");
        sql = sql.substr(0, sql.size()-1) + ")";

        return sql;
    }

    template<class Struct>
    string deleteSqlStr(string conditionStr){
        string sql;
        int status;

        ss.str("");
        ss << "delete from " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status) << " where " << conditionStr;

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string selectSqlStr(string conditionStr){
        string sql;
        int status;

        ss.str("");
        if(conditionStr.empty()){
            ss << "select * from " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status);
        }else{
            ss << "select * from " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status) << " where " << conditionStr;
        }

        sql = ss.str();
        ss.str("");

        return sql;
    }

    template<class Struct>
    string selectCountSqlStr(string conditionStr){
        string sql;
        int status;

        ss.str("");
        if(conditionStr.empty()){
            ss << "select count(1) from " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status);
        }else{
            ss << "select count(1) from " << abi::__cxa_demangle(typeid(Struct).name(), 0, 0, &status) << " where " << conditionStr;
        }

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
     * key1 = key1 and key2 = key2Value
     */
    template<typename ...Args>
    string sqlQueryAndConditions(string joinStr, Args &&...args){
        return SqlQueryAndConditions::sqlQueryAndConditions(joinStr, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    string sqlQueryOrConditions(string joinStr, Args &&...args){
        return SqlQueryOrConditions::sqlQueryOrConditions(joinStr, std::forward<Args>(args)...);
    }

    string sqlQueryLimitConditions(int limit, int offset){
        stringstream ss;
        ss << " limit " << limit << " offset " << offset;
        return ss.str();
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
        static void sqlInsertBindValues(stringstream &ss, Struct &&st){
            using Index = typename MakeIndexes<boost::pfr::tuple_size<typename std::remove_reference<Struct>::type>::value>::type;
            __sqlInsertBindValues(Index{}, ss, forward<decltype(st)>(st));
        }

    private:
        template<typename T>
        static void init(stringstream &ss, T &&t){
            ss << t << ',';
        }
        static void init(stringstream &ss, string str){
            ss << "'" << str << "'" << ',';
        }

        template<int ...Indexes, typename Struct>
        static void __sqlInsertBindValues(IndexSeq<Indexes...> &&, stringstream &ss,  Struct &&st){
            std::initializer_list<int>{(init(ss, boost::pfr::get<Indexes>(st)),0)...};
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
            ss << tmp.substr(0, tmp.size() - 5);
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
            ss << tmp.substr(0, tmp.size() - 4);
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
