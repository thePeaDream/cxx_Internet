#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP
#include <iostream>
#include <string>
//网络四件套
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
class UdpServer
{
public:
    //0.0.0.0表示任意ip地址绑定
    UdpServer(uint16_t port,std::string ip = "0.0.0.0")
    :_port(port)
    ,_ip(ip)
    ,_sock(-1)
    {}
    //初始化服务器
    void initServer()
    {
        //1 创建套接字 —— socket创建通信的一端
        //(1) 头文件：sys/types.h 和sys/socket.h
        //(2) 对传输层做了文件系统级别的封装接口，创建套接字成功返回值，就是文件描述符
        //(3) 网络中的数据经过网线、协议栈后最终被放在服务器进程打开的套接字文件中
        //(4) 接口：int socket(int domain,int type,int protocol)
        //domain:域，将来创建的套接字是哪种类型的 常用：AF_INET ipv4网络通信  AF_LOCAL本地通信 —— 说明是网络通信还是本地通信
        //type:将来创建的套接字的通信种类是什么 面向数据报SOCK_DGRAM —— 在网络中以什么方式通信？流式SOCK_STREAM还是数据报方式
        //protocol:IPPROTO_UDP或IPPROTO_TCP
        _sock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(_sock < 0)
        {
            std::cerr << "创建套接字失败！！！" << std::endl;
            std::cerr << strerror(errno) <<std::endl;
            exit(2);
        }
        std::cerr << "创建套接字成功："<< _sock << std::endl;
        //2 绑定套接字 bind —— 套接字是两台跨网络的主机上面的进程在通信【服务器进程和客户端进程】，ip来标定唯一主机，端口来标识主机上的唯一一个网络进程
        //那服务器ip是多少，端口是多少，套接字还没有这些信息，需要先绑定
        //(1)即：将用户设置的ip和port在内核中和服务器进程强关联
        //(2)接口： bind(int sockfd,const struct sockaddr* addr,socklent_t addrlen)
        //(3)头文件：<sys/socket.h> <netinet/in.h> <arpa/inet.h>
        //int sockfd:将ip端口号和套接字相关联
        //struct sockaddr* addr:通用的sockaddr结构，如果是网络套接字就用sockaddr_in填充ip和port；如果是域间套接字(本地通信)就用sockaddr_un填充服务器文件路径
        //socklent_t addrlen:sockaddr_in的大小或sockaddr_un的大小
        struct sockaddr_in server_addr;
        bzero(&server_addr,sizeof(server_addr));//清零
        //(4)创建的套接字PF_INET表示网络通信，这个填充的结构体，自身是要用来本地通信还是网络通信，也要指定
        server_addr.sin_family = PF_INET;
        //(5)在网络通信时，任何一方想给另一方发信息时，除了自己的消息内容外，也要把自己的ip和端口号告诉对方主机！！！！！方便别人进行回复->先要将数据发到网络！！！->考虑网络字节序
        server_addr.sin_port =  htons(_port);
        //(6)"192.168.1.1"->点分十进制字符串风格的ip地址(给用户看的) 每一个区域取值范围[0,255]就是1byte，4字节，所以表示一个ip地址4字节够了
        //所以一定要互相转换 点分十进制字符串风格的ip<->网络字节序的4字节ip
        //当要进行网络发送时，将点分十进制ip转换成4字节ip；当读取网络里的ip时，4字节ip要转成点分十进制ip字符串
        //先将点分十进制风格的Ip -> 4字节ip,再将4字节主机序列->网络序列 
        server_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(_ip.c_str());
        //(7)bind(int sockfd,const struct sockaddr* addr,socklent_t addrlen)
        if(bind(_sock,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
        {
            std::cerr << "套接字绑定ip和端口号失败！！！" << std::endl;
            std::cerr << strerror(errno) <<std::endl;
            exit(3);
        }
        std::cerr<<"套接字绑定成功！！！"<<std::endl;
        std::cerr<<"服务器初始化成功！！！"<<std::endl;
    }
    //启动服务器
    void startServer()
    {
        char buffer[1024];
        bzero(buffer,sizeof(buffer));
        //网络服务器，永远不退出 -> 服务器进程是常驻进程，一直在内存中存在->当心内存泄漏
        while(true)
        {
            //1 读取数据
            //(1)接口：ssize_t recvfrom(int sockfd,void*buf,size_t len,int flags,struct sockaddr *src_addr,socklen_t *addrlen);
            //(2)参数解析：
            //int sockfd:服务器进程绑定的套接字
            //void* buf和size_t len:读取数据的用户层缓冲区及其大小
            //返回值：本次实际读取的数据字节数
            //flags:读取的方式，默认为0时，以阻塞方式读取
            //后面2个参数都是输出型参数，提取客户端(发送方)的地址信息
            //struct sockaddr *src_addr:发送方的ip和port信息（除了拿到数据，也想知道是谁给我发的消息——src_ip和src_port）
            //socklen_t *addrlen：特殊输入输出型参数,输入时代表peer结构体的大小，输出时代表实际读到peer的大小
            struct sockaddr_in peer;//远端的地址信息
            socklen_t len = sizeof(peer);
            ssize_t s = recvfrom(_sock,buffer,sizeof(buffer)-1,0,
                                (struct sockaddr*)&peer,&len);
            uint16_t peerPort = ntohs(peer.sin_port);
            std::string peerIp = inet_ntoa(peer.sin_addr);
            if(s < 0){
                std::cerr << "读取数据recvfrom发生错误"<<std::endl;
                exit(4);
            }
            else{
                //2 分析和处理数据
                //(1) 数据是什么
                buffer[s] = '\0';
                std::cout << "[" << peerIp << ":" << peerPort << "] send: " << buffer << std::endl;
                //(2) 如果发过来的是一个命令，将执行结果返回
                if(strcasestr(buffer,"rm") || strcasestr(buffer,"rmdir"))//忽略大小写查找子串，只要有rm就返回客户端坏人
                {
                    char comment[] = "坏人,不能乱删!!!";
                    sendto(_sock,comment,strlen(comment),0,(struct sockaddr*)&peer,sizeof(peer));
                    continue;
                }
                //FILE* popen(const char* command,const char* type)
                //第一：执行传入的字符串->在底层建立pipe管道，再fork()出子进程，让子进程执行command命令(调用exec系列函数)
                //第二：FILE*：可以将执行结果通过FILE*类型的指针进行读取
                //例如：ls -l -a
                FILE* pf = popen(buffer,"r");
                if(pf == nullptr){
                    std::cerr << "WARNING:" << "popen发生异常,命令执行失败" << std::endl;
                    char err[] = "指令解析发生异常,请重试";
                    sendto(_sock,err,strlen(err),0,(struct sockaddr*)&peer,sizeof(peer));
                    continue;
                }
                char result[1024];
                std::string cmd_echo;
                while(fgets(result,sizeof(result)-1,pf) != nullptr)
                {
                    //执行结果拼到一个字符串
                    cmd_echo += result;
                }
                //写回结果
                sendto(_sock,cmd_echo.c_str(),cmd_echo.size(),0,(struct sockaddr*)&peer,sizeof(peer)); 
            }   
        }
    }
    ~UdpServer(){
        if(_sock >= 0)
            close(_sock);
    }
private:
    //端口号是一个16位的整数
    uint16_t _port;
    //一个服务器，必须需要ip地址和端口号
    std::string _ip;
    //创建的套接字
    int _sock;
};

#endif