#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP
#include <iostream>
#include <string>
#include <memory>
#include "thread.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <cstring>
#include <cstdio>
using std::cin;
using std::cout;
using std::endl;

//用来获取输入和发送数据的sender线程(需要套接字_sock、目标服务器地址peer)
struct SenderArgs{
    SenderArgs(int sock,const struct sockaddr_in& peer)
    :_sock(sock)
    ,_peer(peer)
    {}
    int _sock;
    struct sockaddr_in _peer;
};
//用来获取服务器数据的recver线程(需要套接字_sock)
struct RecverArgs{
    RecverArgs(int sock)
    :_sock(sock){}
    int _sock;
};

//发送线程执行的函数(读取用户输入，并发送数据给服务器)
static void* senders(void* args)
{
    int sock = ((SenderArgs*)args)->_sock;
    struct sockaddr_in peer = ((SenderArgs*)args)->_peer;
    std::string message;
    while(true)
    {
        cout << "请输入#: ";
        //获取用户输入
        std::getline(cin,message);
        if(message == "quit") exit(0);
        //发送数据给服务器处理
        sendto(sock,message.c_str(),message.size(),0,(struct sockaddr*)&peer,sizeof(peer));
    }
    return nullptr;
}
//接收线程执行的函数(读取服务器发送的信息)
static void* recvers(void* args)
{
    int sock = ((RecverArgs*)args)->_sock;
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    char buffer[1024];
    ssize_t s = 0;
    while(true)
    {
        //阻塞式读取
        s = recvfrom(sock,buffer,sizeof(buffer)-2,0,(struct sockaddr*)&peer,&len);
        buffer[s] = '\n';
        buffer[s+1] ='\0';
        //将读取的数据写到文件中
        FILE* fp = fopen("./serverRecv.txt","a");
        fputs(buffer,fp);
        fclose(fp);
    }
    return nullptr;
}

class UdpClient
{
private:
    int _sock;
    std::string _targetIp;
    uint16_t _targetPort;
public:
    UdpClient(uint16_t targetPort,const std::string& targetIp)
    :_targetIp(targetIp)
    ,_targetPort(targetPort)
    ,_sock(-1)
    {}

    void initClient()
    {
        //1 创建套接字
        _sock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(_sock < 0)
        {
            std::cerr << "创建套接字失败"<<std::endl;
            std::cerr << strerror(errno) << std::endl;
            exit(2);
        }
        std::cerr << "套接字创建成功:"<<_sock << std::endl;
        //2 绑定套接字？
        //客户端不需要手动绑定套接字，一般由os自动绑定(什么时候绑定的？？？)
        //如果手动bind了端口号，那客户端bind了一个固定的端口！！！如果其他客户端提前占用了端口，就会绑定失败（客户端启动冲突）
    }

    void startClient()
    {
        std::string buffer;
        char getMessage[1024];
        struct sockaddr_in peer;
        bzero(&peer,sizeof(peer));
        //填充服务器的sockaddr_in
        peer.sin_family = PF_INET;
        peer.sin_addr.s_addr = inet_addr(_targetIp.c_str()); 
        peer.sin_port = htons(_targetPort);
        //多线程：在等待用户输入时，也能接收服务器消息;
        //一个线程用来获取输入和发送数据(需要套接字_sock、目标服务器地址peer)
        SenderArgs sendArgs(_sock,peer);
        std::unique_ptr<Thread> sender(new Thread(senders,(void*)&sendArgs));
        //另一个线程用来接收服务器数据
        RecverArgs recvArgs(_sock);
        std::unique_ptr<Thread> recver(new Thread(recvers,(void*)&recvArgs));
        
        //启动线程
        sender->Start();
        recver->Start();

        //主线程回收
        sender->Join();
        recver->Join();

        //客户端向服务器发消息
        // while(true)
        // {
            
        //     //套接字 —— 接收线程也要用，发送线程也要用
        //     std::cout << "请输入# ";
        //     std::getline(std::cin,buffer);
        //     if(buffer == "quit") break;

        //     //当client首次发送消息给服务器时，os会自动给client bind它的ip和port
        //     //发送消息给服务器
        //     sendto(_sock,buffer.c_str(),buffer.size(),0,(struct sockaddr*)&peer,sizeof(peer));

        //     struct sockaddr_in temp;
        //     socklen_t len = sizeof(temp);
        //     //阻塞式获取服务器的回复
        //     ssize_t s = recvfrom(_sock,getMessage,sizeof(getMessage),0,(struct sockaddr*)&temp,&len);
        //     if(s < 0)
        //     {
        //         std::cerr << "获取服务器回复发生错误！！！" << std::endl;
        //         continue;
        //     }
        //     else{
        //         getMessage[s] = '\0';
        //         std::cout << "获取服务器发送:" << getMessage << std::endl << std::endl; 
        //     }
        // }
    }
    ~UdpClient()
    {
        close(_sock);
    }

};


#endif