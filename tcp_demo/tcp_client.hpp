#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <strings.h>
#include <string>
#include <unistd.h>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
class TcpClient{
private:
    int _sock;
    uint16_t _targetPort;
    std::string _targetIp;
public:
    TcpClient(uint16_t targetPort,const std::string& targetIp)
    :_sock(-1)
    ,_targetPort(targetPort)
    ,_targetIp(targetIp)
    {}
    void Init()
    {
        //创建套接字
        _sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
        assert(_sock >= 0);
        //连接服务器
        //int connect(int sockfd,const struct sockaddr* addr,socklen_t addrlen)
        struct sockaddr_in peer;
        bzero(&peer,sizeof(peer));
        peer.sin_addr.s_addr = inet_addr(_targetIp.c_str());
        peer.sin_family = PF_INET;
        peer.sin_port = htons(_targetPort);
        //会自动为客户端绑定端口号/ip
        if(connect(_sock,(struct sockaddr*)&peer,sizeof(peer)) < 0){
            cerr << "连接到服务器失败"<<endl;
            exit(1);
        }
        cerr << "客户端初始化完成,连接服务器成功！！！"<<endl;
    }
    void Start()
    {
        char buffer[1024];
        std::string message;
        while(true)
        {
            cout << "请输入# ";
            std::getline(cin,message);
            //send(int sockfd,const void* buf,size_t len,int flags) 基于tcp向目标发消息
            write(_sock,message.c_str(),message.size());
            ssize_t s = read(_sock,buffer,sizeof(buffer)-1);
            if(s > 0)
            {
                buffer[s] = '\0';
                cout << "server# "<< buffer<< endl;
            }
            else if(s == 0)
            {
                cerr << "服务器关闭连接"<<endl;
                break;
            }
            else{
                cerr << "连接发生错误"<<endl;
                exit(2);
            }
        }
    }
};