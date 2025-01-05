#include "ConnectionPool.h"
#include"public.h"
#include<fstream>
#include<unistd.h>
#include<thread>

ConnectionPool& ConnectionPool::GetConnectionPool(const char* path)
{
    static ConnectionPool pool(path);
    return pool;
}
ConnectionPool::ConnectionPool(const char* path)
{
    if(!LoadConf(path))
    {
        exit(EXIT_FAILURE);
    }
    //之后配置项也可能存在错误
    //创建初始数量的连接
    for(int i=0;i<_initSize;++i)
    {
        Connection* conn = new Connection();
        conn->connect(_ip,_port,_username,_password,_dbname);
        conn->resetAliveTime();
        _queue.emplace(conn);
        _connCnt++;
    }

    //启动生产者线程
    std::thread produceThread([&](){
        for(;;)
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _empty.wait(lock,[&]{return _queue.empty()&&_connCnt<_maxSize;});

            Connection *conn=new Connection();
            conn->connect(_ip,_port,_username,_password,_dbname);
            conn->resetAliveTime();
            _queue.emplace(conn);
            _connCnt++;
            _full.notify_one();
        }
    });
    produceThread.detach();

    //管理者线程，扫描空闲时间超过maxIdleTime的空闲连接，并释放资源
    std::thread managerThread([&](){
        for(;;)
        {
            std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
            //扫描队列，释放多余连接
            std::lock_guard<std::mutex> guard(_queueMutex);
            while(_queue.size()>=_maxSize)
            {
                Connection *p=_queue.front();
                if(p->getAliveTime()>=_maxIdleTime*1000)
                {
                    _queue.pop();
                    _connCnt--;
                    delete p;
                }
                else
                {
                    break;
                }
            }
        }
    });
    managerThread.detach();
}
bool ConnectionPool::LoadConf(const char* path)
{
    FILE* fp=fopen(path,"r");
    if(fp==nullptr)
    {
        LOG("配置项路径错误");
        return false;
    }
    
    while(!feof(fp))
    {
        char buff[256]={0};
        fgets(buff,sizeof buff,fp);
        std::string line(buff);
        int idx=line.find('=');
        if(idx==-1)
            continue;
        Trim(line);
        std::string key=line.substr(0,idx);
        Trim(key);
        int idx2=line.find('\n',idx);
        std::string value=line.substr(idx+1,idx2-idx-1);
        Trim(value);
        if('#'==key[0])
            continue;
        if("ip"==key)
        {
            _ip=value;
        }
        else if(key=="port")
        {
            _port=atoi(value.c_str());
        }
        else if(key=="username")
        {
            _username=value;
        }
        else if(key=="password")
        {
            _password=value;
        }
        else if(key=="initSize")
        {
            _initSize=atoi(value.c_str());
        }
        else if(key=="maxSize")
        {
            _maxSize=atoi(value.c_str());
        }
        else if(key=="maxIdleTime")
        {
            _maxIdleTime=atoi(value.c_str());
        }
        else if(key=="maxConnTimeout")
        {
            _maxConnTimeout=atoi(value.c_str());
        }
        else if(key=="dbname")
        {
            _dbname=value;
        }
        else
        {
            LOG("不存在的配置项");
            continue;
        }
    }
    return true;
}
void ConnectionPool::Trim(std::string& str)
{
    int idx = str.find_first_not_of(' ');
        if (idx != std::string::npos)
        {
            str.erase(0, idx);
            idx = str.find_last_not_of(' ');
            //if (idx != std::string::npos)
            str.erase(idx + 1);
        }
        else
            str.clear();
}
std::shared_ptr<Connection> ConnectionPool::GetxConnection()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    if(_queue.empty())
    {
        if(!_full.wait_for(lock,std::chrono::milliseconds(_maxConnTimeout),[&](){
            return !_queue.empty();
            }
            ));
        {
            LOG("获取空闲连接超时!");
            return nullptr;
        }
    }
    std::shared_ptr<Connection> p(_queue.front(),[&](Connection *conn){
        std::lock_guard<std::mutex> lock(_queueMutex);
        conn->resetAliveTime();
        _full.notify_one();
        _queue.emplace(conn);
    });
    _queue.pop();
    _empty.notify_all();
    return p;
}