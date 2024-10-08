//网络四件套
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <memory>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h> //信号忽略
#include <unistd.h>
#include <pthread.h>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
//多线程版本 线程回调函数 参数
class TcpServerServiceThreadArgs
{
public:
    TcpServerServiceThreadArgs(int serviceSock,uint16_t clientPort,const std::string& clientIp)
    :_serviceSock(serviceSock)
    ,_clientPort(clientPort)
    ,_clientIp(clientIp)
    {}
    int _serviceSock;
    uint16_t _clientPort;
    std::string _clientIp;
};

class TcpServer
{
private:
    int _listensock;
    std::string _ip;
    uint16_t _port;
public:
    TcpServer(uint16_t port,const std::string& ip="0.0.0.0")
    :_listensock(-1)
    ,_ip(ip)
    ,_port(port)
    {}
    void InitServer()
    {
        //1 创建套接字——进程和文件 流式套接字
       _listensock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_listensock < 0){
            cerr << "创建套接字失败:"<<strerror(errno) << endl;
            exit(1);
        }
        cerr << "创建套接字成功:"<<_listensock<< endl;
        //2 bind——文件和网络
        struct sockaddr_in local;
        bzero(&local,sizeof(local));
        local.sin_family = PF_INET;
        local.sin_addr.s_addr = inet_addr(_ip.c_str());
        local.sin_port = htons(_port);
        if(bind(_listensock,(struct sockaddr*)&local,sizeof(local)) < 0)
        {
            cerr << "socket绑定失败"<<strerror(errno)<<endl;
            exit(2);
        }
        //3 TCP是面向连接的，当我们正式通信时，需要先建立连接
        //所以服务器要处于等待被连接的状态！！！
        //将套接字设置为监听状态：listen(int sockfd,int backlog//全连接队列长度,不能过大过小);
        if(listen(_listensock,10) < 0)
        {
             cerr << "socket设置监听失败"<<strerror(errno)<<endl;
             exit(3);
        }
        cerr << "初始化套接字成功!!!!!" << endl;
    }
    void StartServer()
    {
        //signal(SIGCHLD, SIG_IGN);//子进程退出时，会向父进程发送SIGCHLD信号，主动忽略——子进程退出时会自动释放自己的僵尸状态
        //4 获取连接,没人来连接就会阻塞 
        //int accept(int sockfd,struct sockaddr* client,socklent* addrlen)
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        while(true)
        {
            //如果accept成功，返回一个合法套接字(文件描述符)
            //和监听套接字区别？监听套接字：饭店外面拉客的人  accept分配的套接字：酒店的服务员（）
            //监听套接字的职责只有一个：通过accept获得一个新连接；真正提供网络通信服务的是serviceSock
            int serviceSock = accept(_listensock,(sockaddr*)&client,&len);
            if(serviceSock < 0)
            {
                //拉客失败/获取连接失败,不致命
                cerr << "获取一个连接发生错误" << endl;
                continue;
            }
            //5 获取连接成功了，开始进行通信服务
            uint16_t clientPort = ntohs(client.sin_port);
            std::string clientIp = inet_ntoa(client.sin_addr);
            cerr << "连接成功,当前连接:"<<"["<<clientIp<<":"<<clientPort<<"]"<< "#"<< serviceSock << endl;

            //version 3 --多线程版
            pthread_t tid;
            TcpServerServiceThreadArgs* args=new TcpServerServiceThreadArgs(serviceSock,clientPort,clientIp);
            //线程创建成功一定是参数传成功了，如果args不在堆上开空间，可能线程还没运行，args内部参数又被改变了，线程不安全，传的参数尽量在堆上开空间
            pthread_create(&tid,nullptr,serviceThread,(void*)args);

            
            //version 2.1 --多进程版 不采用忽略SIGCHLD
            // pid_t id = fork();
            // //子进程内部：父进程比子进程先退出，导致孙子进程是孤儿进程，由系统1号进程领养回收资源
            // if(id == 0)
            // {
            //     close(_listensock);
            //     //子进程立刻退出
            //     if(fork() > 0)
            //          exit(0);
            //     //孙子进程（孤儿进程）
            //     service(serviceSock,clientPort,clientIp);
            //     exit(0);
            // }
            // //父进程可以立刻进程等待 回收子进程
            // waitpid(id,nullptr,0);
            // close(serviceSock);

            //version 1 -- 单进程循环版 -- 只能一次处理一个客户端，处理完一个，才能处理下一个，不能使用
            //service(serviceSock,clientPort,clientIp);
            
            //version 2 -- 多进程版
            //创建子进程，让子进程给新的连接提供服务，子进程能不能打开父进程曾经打开的文件fd呢？
            //能，子进程会继承父进程的文件描述符表！！！！      
            // pid_t id = fork();
            // assert(id!=-1);
            // if(id == 0)
            // {
            //     //子进程 会继承父进程打开的文件和fd，子进程不需要知道监听socket，尽量让进程关闭掉不需要的套接字
            //     close(_listensock);
            //     service(serviceSock,clientPort,clientIp);
            //     exit(0);
            // }
            // //父进程只保留_listenSock即可,如果不关闭serviceSock,可用的文件描述符会越来越少
            // close(serviceSock);
        }
    }
    //连接成功后的服务逻辑
    static void service(int serviceSock,uint16_t clientPort,const std::string& clientIp)
    {
        char buffer[1024];
        //write 和 read可以直接被使用！！！
        //读取客户端发送数据
        while(true)
        {
            ssize_t s = read(serviceSock,buffer,sizeof(buffer)-1);
            cerr << "线程读取完成"<<endl;
            if(s>0)
            {
                buffer[s] = 0;
                cout << clientIp<<":"<<clientPort<<"# "<<buffer << endl;
            }
            else if(s == 0)//对端关闭连接
            {
                cerr << "对端["<<clientIp<<":"<<clientPort<<"]关闭连接,我也要关"<<endl;
                break;
            }
            else{
                cerr << "读取套接字信息发生错误,本次连接终止！！！"<<endl;
                break;
            }
            write(serviceSock,buffer,strlen(buffer));
        }
    }
    static void* serviceThread(void* args)
    {
        //线程分离，主线程不需要等待释放该线程资源
        pthread_detach(pthread_self());
        TcpServerServiceThreadArgs* tdArgs = static_cast<TcpServerServiceThreadArgs*>(args);
        service(tdArgs->_serviceSock,tdArgs->_clientPort,tdArgs->_clientIp);
        close(tdArgs->_serviceSock);
        delete tdArgs;
        return nullptr;
    }
    ~TcpServer()
    {
        if(_listensock >= 0)
            close(_listensock);
    }
};