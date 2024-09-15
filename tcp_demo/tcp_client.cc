#include "tcp_client.hpp"
#include <memory>
using namespace std;
static void usage(const string& proc)
{
    cout << "Usage:" << proc << " targetPort targetIp"<<endl;
}
// ./tcp_client targetPort targetIp
int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
        exit(0);
    }
    uint16_t targetPort = atoi(argv[1]);
    string targetIp = argv[2];
    std::unique_ptr<TcpClient> client(new TcpClient(targetPort,targetIp));
    client->Init();
    client->Start();
    return 0;
}