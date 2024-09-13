#include <memory>
#include "udp_client.hpp"
using namespace std;
//使用手册：用static修饰，只在本文件可见
static void usage(const string& proc)
{
    //发送命令，返回命令执行结果
    cout << "Usage:"<< proc << " targetIp targetPort"<<endl;
}

//./udp_client targetIp targetPort
//./udp_client 127.0.0.1 8080 
//127.0.0.1是本地环回，将来client和server在通信时如果使用本地环回，client和server发送数据只在本地协议栈中进行数据流动，不会把数据发到网络中（仅仅在协议栈走一遭）
//如果客户端和服务器在同一台机器上，将来client给server发信息，
//client发的消息经过协议栈(应用层、传输层、网络层、数据链路层)，直接在协议栈底部依次交付给上层(数据链路层、网络层、传输层、应用层) 
//本地网络服务器的测试
int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
        exit(1);
    }
    string targetIp = argv[1];
    uint16_t targetPort = atoi(argv[2]);
    unique_ptr<UdpClient> client(new UdpClient(targetPort,targetIp));
    client->initClient();
    client->startClient();
    return 0;
}