//
// Created by ubuntu on 10/13/22.
//

#include "SqlConnectionSqlite.h"

SqlConnectionSqlite::SqlConnectionSqlite(string dbPath) : db(dbPath) {

}

SqlConnectionSqlite::~SqlConnectionSqlite() {
    db.close();
}

bool SqlConnectionSqlite::begin() {
    return db.begin();
}

bool SqlConnectionSqlite::commit() {
    return db.commit();
}

bool SqlConnectionSqlite::rollBack() {
    return db.rollBack();
}

bool SqlConnectionSqlite::execute(string sql) {
    cout << sql << endl;
    return db.execute(sql);
}

string SqlConnectionSqlite::getLastError() {
    return db.getLastError();
}

int SqlConnectionSqlite::getLastErrorCode() {
    return db.getLastErrorCode();
}

bool SqlConnectionSqlite::queryCountSql(int &count, string sql) {
    cout << sql << endl;
    do{
        if(!db.query(sql)){
            break;
        }
        if(!db.next()){
            break;
        }
        count = db.get<int>(0);
        return true;
    }while(false);

    return false;
}

