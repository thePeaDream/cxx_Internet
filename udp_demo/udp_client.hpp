#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <cstring>
#include <cstdio>
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
        //客户端向服务器发消息
        while(true)
        {
            std::cout << "请输入# ";
            std::getline(std::cin,buffer);
            if(buffer == "quit") break;

            //当client首次发送消息给服务器时，os会自动给client bind它的ip和port
            //发送消息给服务器
            sendto(_sock,buffer.c_str(),buffer.size(),0,(struct sockaddr*)&peer,sizeof(peer));

            struct sockaddr_in temp;
            socklen_t len = sizeof(temp);
            //阻塞式获取服务器的回复
            ssize_t s = recvfrom(_sock,getMessage,sizeof(getMessage),0,(struct sockaddr*)&temp,&len);
            if(s < 0)
            {
                std::cerr << "获取服务器回复发生错误！！！" << std::endl;
                continue;
            }
            else{
                getMessage[s] = '\0';
                std::cout << "获取服务器发送:" << getMessage << std::endl << std::endl; 
            }
        }
    }
    ~UdpClient()
    {
        close(_sock);
    }

};


#endif