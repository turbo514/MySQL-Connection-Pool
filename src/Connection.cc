#include"public.h"
#include "Connection.h"
#include<iostream>

Connection::Connection()
{
    _conn=mysql_init(nullptr);
    if(!_conn)
    {
        LOG("MySQL initialization error!");
    }
}
Connection::~Connection()
{
    if(_conn)
    {
        mysql_close(_conn);
    }
}
bool Connection::connect(std::string ip,unsigned short port,std::string user,std::string pwd,std::string dbname)
{
    MYSQL* p=mysql_real_connect(_conn,ip.c_str(),user.c_str(),pwd.c_str(),dbname.c_str(),port,nullptr,0);
    if (!p) 
    {
        LOG("MySQL connection error: " + std::string(mysql_error(_conn)));
    }
    return p!=nullptr;
}
bool Connection::update(std::string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("update fail:"+sql);
        return false;
    }
    return true;
}
MYSQL_RES* Connection::query(std::string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("查询失败:"+sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

void Connection::resetAliveTime()
{
    _aliveTime=clock();
}

clock_t Connection::getAliveTime()
{
    return clock()-_aliveTime;
}
