#pragma once
//连接池模块
#include<string>
#include<queue>
#include"Connection.h"
#include<mutex>
#include<atomic>
#include<memory>
#include<condition_variable>

class ConnectionPool
{
public:
    static ConnectionPool& GetConnectionPool(const char* path=nullptr);
    // 从连接池获取一个可用的空闲连接
    std::shared_ptr<Connection> GetxConnection();
private:
    std::queue<Connection*> _queue;
    std::mutex _queueMutex;
    std::string _ip;
    unsigned short _port;
    //数据库名称
    std::string _dbname;
    //用户名，密码
    std::string _username,_password;
    //连接池起始连接量
    int _initSize;
    //连接池最大连接量
    int _maxSize;
    //连接池最大空闲时间，以秒为单位
    int _maxIdleTime;
    //连接超时时间，以毫秒为单位
    int _maxConnTimeout;
    //目前启动的连接数
    std::atomic_int _connCnt;
    //可用线程信号量(新建的，不包括基础线程)
    std::condition_variable _empty,_full;
    bool LoadConf(const char* path);
    ConnectionPool(const char* path);
    ConnectionPool(const ConnectionPool&)=delete;
    ConnectionPool(ConnectionPool&&)=delete;

    void Trim(std::string& str);
};