#include "tcp_server.hpp"
#include <memory>
using namespace std;
static void usage(const string& proc)
{
    cout << "Usage:" << proc << " port"<<endl;
}
// ./tcp_server port
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        exit(0);
    }
    uint16_t port = atoi(argv[1]);
    std::unique_ptr<TcpServer> server(new TcpServer(port));
    server->InitServer();
    server->StartServer();
    return 0;
}