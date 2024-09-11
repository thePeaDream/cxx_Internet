#include <memory>
#include "udp_server.hpp"
using namespace std;

//使用手册(该手册只在该文档中可见)
static void usage(const string& proc)
{
    cout << "Usage:" << proc << " ip port" << endl;
}
//./udp_server ip port,客户端通过ip地址和端口号，向服务器发起数据请求
int main(int argc,char* argv[])
{
    if(argc != 3){
        //参数不够
        usage(argv[0]);
        exit(1);
    }
    string ip = argv[1];
    //注意转成整数
    uint16_t port = atoi(argv[2]);

    unique_ptr<UdpServer> server(new UdpServer(port,ip));
    server->initServer();
    server->startServer();
    return 0;
}