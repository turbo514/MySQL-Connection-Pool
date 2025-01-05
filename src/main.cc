#include"Connection.h"
#include<iostream>
#include"ConnectionPool.h"
#include<thread>
//#define POOL
//#define ONE
int main(int argc,char* argv[])
{
    if(argc<2)
    {
        std::cout<<"example:./test path\n";
        exit(EXIT_FAILURE);
    }
    //const ConnectionPool& pool=ConnectionPool::GetConnectionPool(argv[1]);
    std::string sql="insert into users(name,age,sex) values('li si',30,'male')";
    #ifdef POOL
        ConnectionPool& pool=ConnectionPool::GetConnectionPool(argv[1]);
    #endif



#ifdef ONE
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0;i<1000;i++)
    {
        #ifndef POOL
            Connection conn;
            conn.connect("127.0.0.1",3306,"root","2310net","chat");
            conn.update(sql);
        #else
            std::shared_ptr<Connection> conn=pool.GetxConnection();
            conn->update(sql);
        #endif
    }
#else
    auto func = [&](){
        for(int i=0;i<10000/4;i++)
        {
        #ifndef POOL
            Connection conn;
            conn.connect("127.0.0.1",3306,"root","2310net","chat");
            conn.update(sql);
        #else
            std::shared_ptr<Connection> conn=pool.GetxConnection();
            conn->update(sql);
        #endif
    }};
    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1(func);
    std::thread t2(func);
    std::thread t3(func);
    std::thread t4(func);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
#endif
    auto end = std::chrono::high_resolution_clock::now(); // 结束计时
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " ms" << std::endl;
}