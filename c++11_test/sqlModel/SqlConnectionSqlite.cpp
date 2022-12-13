//
// Created by ubuntu on 10/13/22.
//

#include "SqlConnectionSqlite.h"

SqlConnectionSqlite::SqlConnectionSqlite(string dbPath) : db(dbPath) {
    dbName = boost::filesystem::path(dbPath).filename().string();
}

SqlConnectionSqlite::~SqlConnectionSqlite() {
    db.close();
}

bool SqlConnectionSqlite::begin() {
    cout << dbName << ":\t" << "begin" << endl;
    return db.begin();
}

bool SqlConnectionSqlite::commit() {
    cout << dbName << ":\t" << "commit" << endl;
    return db.commit();
}

bool SqlConnectionSqlite::rollBack() {
    cout << dbName << ":\t" << "rollback" << endl;
    return db.rollBack();
}

bool SqlConnectionSqlite::prepare(string sql) {
    cout << dbName << ":\t" << sql << endl;
    return db.prepare(sql);
}

bool SqlConnectionSqlite::execute(string sql) {
    cout << dbName << ":\t" << sql << endl;
    return db.execute(sql);
}

string SqlConnectionSqlite::getLastError() {
    return db.getLastError();
}

int SqlConnectionSqlite::getLastErrorCode() {
    return db.getLastErrorCode();
}

bool SqlConnectionSqlite::queryCountSql(int &count, string sql) {
    cout << dbName << ":\t" << sql << endl;
    do{
        if(!db.query(sql)){
            break;
        }
        if(!db.next()){
            break;
        }
        count = db.get<int>(0);
        while(db.next());

        return true;
    }while(false);

    return false;
}

