//
// Created by root on 8/16/21.
//

#ifndef SQLITE3_SQLITE_CPP_H
#define SQLITE3_SQLITE_CPP_H

#include <string>
#include <map>
#include <type_traits>
#include <typeinfo>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/pfr.hpp>

#include "../sqlite3-cmake-master/src/sqlite3.h"
#include "./non_copyable.h"

using namespace std;

struct blob
{
    const char *pBuf;
    size_t size;

    // sql获取blob数据后转换为string防止sqlite3_stmt重置后，pBuf指向非法内存
    string toString(){
        return {pBuf, size};
    }
};

class SqliteCpp : NonCopyable {
public:
    SqliteCpp(const string &fileName){
        open(fileName);
    }

    ~SqliteCpp(){
        close();
    }

    bool open(const string &fileName){
        auto dir = boost::filesystem::path(fileName).parent_path().string();
        if(!boost::filesystem::exists(dir)){
            boost::filesystem::create_directories(dir);
        }
        if(sqlite3_threadsafe() != SQLITE_CONFIG_MULTITHREAD){
            sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
        }

        if(m_dbHandle != nullptr)
            return true;

        m_code = sqlite3_open(fileName.c_str(), &m_dbHandle);

        if(m_code == SQLITE_OK){
            sqlite3_exec(m_dbHandle, "PRAGMA foreign_keys = 1;", 0,0,0);
            sqlite3_exec(m_dbHandle, "PRAGMA auto_vacuum = 0;", 0,0,0);
        }else{
            m_dbHandle = nullptr;
        }

        return m_code == SQLITE_OK;
    }

    bool close(){
        if(m_dbHandle == nullptr)
            return true;
        do{
            m_code = sqlite3_finalize(m_statement);
            if(m_code != SQLITE_OK)
                break;
            m_statement = nullptr;

            m_code = closeDbHandle();
            if(m_code != SQLITE_OK)
                break;
            m_dbHandle = nullptr;
        }while(false);
        return m_code == SQLITE_OK;
    }

    bool begin(){
        return execute("BEGIN");
    }

    bool rollBack(){
        return execute("ROLLBACK");
    }

    bool commit(){
        return execute("COMMIT");
    }

    bool execute(const string &sql){
        m_code = sqlite3_exec(m_dbHandle, sql.c_str(), nullptr, nullptr, nullptr);
        return m_code == SQLITE_OK;
    }

    template<typename ...Args>
    bool execute(const string &sql, Args&&...args){
        if(!prepare(sql))
            return false;
        return executeArgs(forward<Args>(args)...);
    }

    bool prepare(const string &sql){
        if(m_statement){
            m_code = sqlite3_finalize(m_statement);
            if(m_code != SQLITE_OK)
                return false;
            m_statement = nullptr;
        }
        m_code = sqlite3_prepare_v2(m_dbHandle, sql.c_str(), -1, &m_statement, NULL);
        return m_code == SQLITE_OK;
    }

    template<class Struct>
    bool executeStruct(Struct &&st, int startIndex){
        if(SqlInsertBindValues::sqlInsertBindValues(this, std::forward<Struct>(st), startIndex) != SQLITE_OK)
            return false;
        m_code = sqlite3_step(m_statement);
        sqlite3_reset(m_statement);
        return m_code == SQLITE_DONE;
    }

    template<typename ...Args>
    bool executeArgs(Args&&...args){
        if(bindParams(1, forward<Args>(args)...) != SQLITE_OK)
            return false;
        m_code = sqlite3_step(m_statement);
        sqlite3_reset(m_statement);
        return m_code == SQLITE_DONE;
    }

    bool query(const string &sql){
        return prepare(sql);
    }

    template<typename ...Args>
    bool query(const string &sql, Args&&...args){
        return prepare(sql) && bindParams(1, forward<Args>(args)...)==SQLITE_OK;
    }

    bool next(){
        m_code = sqlite3_step(m_statement);
        if(m_code == SQLITE_DONE)
            sqlite3_reset(m_statement);
        return m_code == SQLITE_ROW;
    }

    template<typename R>
    R get(int index){
        return columnValue<R>(index);
    }

    int getLastErrorCode(){
        return m_code;
    }
    string getLastError(){
        return sqlite3_errstr(m_code);
    }

protected:
    int closeDbHandle(){
        int code = sqlite3_close(m_dbHandle);
        while (code == SQLITE_BUSY){
            code = SQLITE_OK;
            sqlite3_stmt *stmt = sqlite3_next_stmt(m_dbHandle, NULL);
            if(stmt == NULL)
                break;
            code = sqlite3_finalize(stmt);
            if(code == SQLITE_OK){
                code = sqlite3_close(m_dbHandle);
            }
        }
        return code;
    }

    int bindParams(int current){
        return SQLITE_OK;
    }
    template<typename First, typename ...Rest>
    int bindParams(int current, First&&first, Rest&&...rest){
        bindValue(current, forward<First>(first));
        if(m_code != SQLITE_OK)
            return m_code;
        bindParams(current+1, forward<Rest>(rest)...);
        return m_code;
    }

    template<typename T, typename enable_if<is_floating_point<typename decay<T>::type>::value>::type* = nullptr>
    typename enable_if<is_floating_point<T>::value>::type bindValue(int current, T && t){
        m_code = sqlite3_bind_double(m_statement, current, forward<T>(t));
    }
    template<typename T, typename enable_if<is_integral<typename decay<T>::type>::value>::type* = nullptr>
    void bindValue(int current, T && t){
        bindIntValue(current, forward<T>(t));
    }
    template<typename T, typename enable_if<is_same<typename decay<T>::type, int64_t>::value || is_same<typename decay<T>::type, uint64_t>::value>::type* = nullptr>
    void bindIntValue(int current, T && t){
        m_code = sqlite3_bind_int64(m_statement, current, forward<T>(t));
    }
    template<typename T, typename enable_if<!is_same<typename decay<T>::type, int64_t>::value && !is_same<typename decay<T>::type, uint64_t>::value>::type* = nullptr>
    void bindIntValue(int current, T && t){
        m_code = sqlite3_bind_int(m_statement, current, forward<T>(t));
    }
    template<typename T, typename enable_if<is_same<typename decay<T>::type, string>::value>::type* = nullptr>
    void bindValue(int current, T && t){
        m_code = sqlite3_bind_text(m_statement, current, t.c_str(), t.size(), SQLITE_TRANSIENT);
    }
    template<typename T, typename enable_if<is_same<typename decay<T>::type, char*>::value || is_same<typename decay<T>::type, const char*>::value>::type* = nullptr>
    void bindValue(int current, T && t){
        m_code = sqlite3_bind_text(m_statement, current, t, strlen(t), SQLITE_TRANSIENT);
    }
    template<typename T, typename enable_if<is_same<typename decay<T>::type, blob>::value>::type* = nullptr>
    void bindValue(int current, T && t){
        m_code = sqlite3_bind_blob(m_statement, current, t.pBuf, t.size, SQLITE_TRANSIENT);
    }
    template<typename T, typename enable_if<is_same<typename decay<T>::type, nullptr_t>::value>::type* = nullptr>
    void bindValue(int current, T && t){
        m_code = sqlite3_bind_null(m_statement, current);
    }

    template<typename R, typename enable_if<is_floating_point<R>::value>::type* = nullptr>
    R columnValue(int index){
        if(sqlite3_column_type(m_statement, index) != SQLITE_FLOAT){
            cout << __FILE__ << " " << __LINE__ << ": type error" << endl;
            while(next());
            throw bad_cast();
        }

        return sqlite3_column_double(m_statement, index);
    }
    template<typename R, typename enable_if<is_integral<R>::value>::type* = nullptr>
    R columnValue(int index){
        int type = sqlite3_column_type(m_statement, index);
        if(type != SQLITE_INTEGER){
            cout << __FILE__ << " " << __LINE__ << ": type error" << endl;
            while(next());
            throw bad_cast();
        }

        return columnIntValue<R>(index);
    }
    template<typename R, typename enable_if<is_same<typename decay<R>::type, int64_t>::value || is_same<typename decay<R>::type, uint64_t>::value>::type* = nullptr>
    R columnIntValue(int index){
        return sqlite3_column_int64(m_statement, index);
    }
    template<typename R, typename enable_if<!is_same<typename decay<R>::type, int64_t>::value && !is_same<typename decay<R>::type, uint64_t>::value>::type* = nullptr>
    R columnIntValue(int index){
        return sqlite3_column_int(m_statement, index);
    }
    template<typename R, typename enable_if<is_same<R, string>::value>::type* = nullptr>
    R columnValue(int index){
        auto type = sqlite3_column_type(m_statement, index);
        if(type != SQLITE_TEXT && type != SQLITE_BLOB){
            cout << __FILE__ << " " << __LINE__ << ": type error" << endl;
            while(next());
            throw bad_cast();
        }

        auto tmp = sqlite3_column_text(m_statement, index);
        size_t size = sqlite3_column_bytes(m_statement, index);
        return string{(char*)tmp, size};
    }
    template<typename R, typename enable_if<is_same<R, blob>::value>::type* = nullptr>
    R columnValue(int index){
        auto type = sqlite3_column_type(m_statement, index);
        if(type != SQLITE_BLOB && type != SQLITE_TEXT){
            cout << __FILE__ << " " << __LINE__ << ": type error" << endl;
            while(next());
            throw bad_cast();
        }

        auto tmp = sqlite3_column_blob(m_statement, index);
        size_t size = sqlite3_column_bytes(m_statement, index);
        return blob{(char*)tmp, size};
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
        static int sqlInsertBindValues(SqliteCpp *db, Struct &&st, int startIndex){
            using Index = typename MakeIndexes<boost::pfr::tuple_size<typename std::remove_reference<Struct>::type>::value>::type;
            return __sqlInsertBindValues(Index{}, db, forward<decltype(st)>(st), startIndex);
        }

    private:
        template<typename T>
        static void init(SqliteCpp *db, int index, T &&t, int &value, int startIndex){
            if(value == SQLITE_OK && index >= startIndex){
                db->bindParams(index - startIndex + 1, t);
            }
        }

        template<int ...Indexes, typename Struct>
        static int __sqlInsertBindValues(IndexSeq<Indexes...> &&, SqliteCpp *db,  Struct &&st, int startIndex){
            int value = SQLITE_OK;
            std::initializer_list<int>{(init(db, Indexes, boost::pfr::get<Indexes>(st), value, startIndex),0)...};
            return value;
        }
    };

private:
    sqlite3 *m_dbHandle{nullptr};
    sqlite3_stmt *m_statement{nullptr};
    int m_code;
};


#endif //SQLITE3_SQLITE_CPP_H
