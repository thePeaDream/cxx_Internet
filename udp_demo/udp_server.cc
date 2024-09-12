#include <memory>
#include "udp_server.hpp"
using namespace std;

//使用手册(该手册只在该文档中可见)
static void usage(const string& proc)
{
    cout << "Usage:" << proc << " port" << endl;
}
//./udp_server port
//云服务器无法绑定公网ip，云服务器的ip是虚拟出来的
//server也不推荐绑定确定的ip,推荐使用任意ip的方案 INADDR_ANY(就是0，代表让服务器在工作过程中，可以从任意ip中获取数据)
//有时候计算机不只配了一张网卡，每张网卡都有不同的ip
//如果在服务器端绑定了某个具体ip，服务器只能收到发给该ip的消息
int main(int argc,char* argv[])
{
    if(argc != 2){
        //参数不够
        usage(argv[0]);
        exit(1);
    }
    //string ip = argv[1];
    //注意转成整数
    uint16_t port = atoi(argv[1]);

    unique_ptr<UdpServer> server(new UdpServer(port));
    server->initServer();
    server->startServer();
    return 0;
}