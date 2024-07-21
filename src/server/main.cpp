#include "chatserver.hpp"
#include <iostream>
#include <signal.h>
#include "chatservice.hpp"
using namespace std;
void resetHandler(int){
    ChatService::getChatService()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
     if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);
    
    EventLoop loop;
    InetAddress adress(ip, port);
    ChatServer chatserver(&loop, adress, "chatserver");
    chatserver.start();
    loop.loop();
    return 0;
}