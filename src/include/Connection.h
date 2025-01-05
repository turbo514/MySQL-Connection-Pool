#pragma once
//Mysql数据库的操作
#include<mysql.h>
#include<string>
#include<ctime>

class Connection
{
public:
    Connection();
    ~Connection();
    bool connect(std::string ip,unsigned short port,std::string user,std::string pwd,std::string dbname);
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
    //重置连接的起始空闲时间点
    void resetAliveTime();
    //获取连接的存活时间
    clock_t getAliveTime();
private:
    //与MySQL Server的连接
    MYSQL* _conn;
    //进入空闲状态后的存活时间
    clock_t _aliveTime;
};