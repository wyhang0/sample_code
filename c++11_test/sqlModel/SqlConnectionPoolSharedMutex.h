//
// Created by ubuntu on 10/14/22.
//

#ifndef C__11_TEST_SQLCONNECTIONPOOLSHAREDMUTEX_H
#define C__11_TEST_SQLCONNECTIONPOOLSHAREDMUTEX_H

#include <shared_mutex>
#include <mutex>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * sql连接随用随取，不要长时间占用一个连接，否则会由于读写锁未释放造成后续获取连接失败.
 * 使用读写锁控制一写多读，获取写锁优先级高
 * @tparam SqlConnectionPoolSharedMutex
 */
template<class SqlConnection>
class SqlConnectionPoolSharedMutex : public boost::enable_shared_from_this<SqlConnectionPoolSharedMutex<SqlConnection>>, public boost::noncopyable {
public:
    template<typename ...Args>
    SqlConnectionPoolSharedMutex(int num, Args &&...args){
        for(int i=0; i < num; i++){
            boost::shared_ptr<SqlConnection> sqlConnection(new SqlConnection(std::forward<Args>(args)...), [](SqlConnection *p){}); //在析构函数释放内存
            sqlConnections.push(sqlConnection);
        }
    }
    ~SqlConnectionPoolSharedMutex(){
        while (!sqlConnections.empty()){
            auto sqlConnection = sqlConnections.front();
            sqlConnections.pop();
            delete sqlConnection.get();
        }
    }

    boost::shared_ptr<SqlConnection> getExecuteConnection(int millSeconds = 0){
        do{
            if(millSeconds <= 0){
                millSeconds = INT_MAX;
            }

            {
                std::unique_lock<std::mutex> locker(mutex);
                //要获取写锁，计数器加一
                getExecuteConnectionCount++;
                if (sqlConnections.empty()) {
                    if (!execCond.wait_for(locker, std::chrono::milliseconds(millSeconds), [this]() { return !sqlConnections.empty(); })) {
                        break;
                    }
                }
            }

            if(sharedTimedMutex.try_lock_for(std::chrono::milliseconds(millSeconds))){
                boost::shared_ptr<SqlConnection> tmp;
                {
                    std::lock_guard<std::mutex> locker(mutex);
                    tmp = sqlConnections.front();
                    sqlConnections.pop();
                }
                auto mirror = this->shared_from_this();
                return boost::shared_ptr<SqlConnection>(tmp.get(), [mirror, this](SqlConnection *p){
                    {
                        std::lock_guard<std::mutex> locker(mutex);
                        sqlConnections.push(boost::shared_ptr<SqlConnection>(p, [](SqlConnection *p) {}));
                        sharedTimedMutex.unlock();
                        //释放完写锁计数减一
                        getExecuteConnectionCount--;
                        if(getExecuteConnectionCount > 0){
                            execCond.notify_one();
                        }else{
                            queryCond.notify_all();
                        }
                    }
                });
            }
        }while(false);

        //获取写锁失败计数减一
        {
            std::lock_guard<std::mutex> locker(mutex);
            getExecuteConnectionCount--;
            if(getExecuteConnectionCount > 0){
                execCond.notify_one();
            }else{
                queryCond.notify_all();
            }
        }


        return boost::shared_ptr<SqlConnection>();
    }

    boost::shared_ptr<SqlConnection> getQueryConnection(int millSeconds = 0){
        do{
            if(millSeconds <= 0){
                millSeconds = INT_MAX;
            }
            {
                std::unique_lock<std::mutex> locker(mutex);
                if (sqlConnections.empty() or getExecuteConnectionCount > 0) {
                    if (!queryCond.wait_for(locker, std::chrono::milliseconds(millSeconds), [this]() { return !sqlConnections.empty() && getExecuteConnectionCount == 0; })) {
                        break;
                    }
                }
            }

            if(sharedTimedMutex.try_lock_shared_for(std::chrono::milliseconds(millSeconds))){
                boost::shared_ptr<SqlConnection> tmp;
                {
                    std::lock_guard<std::mutex> locker(mutex);
                    tmp = sqlConnections.front();
                    sqlConnections.pop();
                }
                auto mirror = this->shared_from_this();
                return boost::shared_ptr<SqlConnection>(tmp.get(), [mirror, this](SqlConnection *p){
                    {
                        std::lock_guard<std::mutex> locker(mutex);
                        sqlConnections.push(boost::shared_ptr<SqlConnection>(p, [](SqlConnection *p) {}));
                        sharedTimedMutex.unlock_shared();
                        if(getExecuteConnectionCount > 0){
                            execCond.notify_one();
                        }else{
                            queryCond.notify_all();
                        }
                    }
                });
            }
        }while(false);

        {
            std::lock_guard<std::mutex> locker(mutex);
            if(getExecuteConnectionCount > 0){
                execCond.notify_one();
            }else{
                queryCond.notify_all();
            }
        }

        return boost::shared_ptr<SqlConnection>();
    }

private:
    std::queue<boost::shared_ptr<SqlConnection>> sqlConnections;

    //确保只能获取一个sql写连接，可以获取多个sql读连接
    int getExecuteConnectionCount = 0;
    std::shared_timed_mutex sharedTimedMutex;

    //确保sqlConnections队列pop push操作线程安全
    std::mutex mutex;
    std::condition_variable execCond;
    std::condition_variable queryCond;
};


#endif //C__11_TEST_SQLCONNECTIONPOOLSHAREDMUTEX_H
