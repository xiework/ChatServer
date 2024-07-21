#include "chatserver.hpp"
#include <functional>
#include "json.hpp"
#include "chatservice.hpp"

using namespace std;
using namespace placeholders;

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& address, const std::string& name)
    :m_server(loop, address, name), m_loop(loop)
{
    //设置连接和断开事件的回调函数
    m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    //设置读写事件的回调函数
    m_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1,_2,_3));

    //设置线程池数量
    m_server.setThreadNum(4);
}

void ChatServer::start()
{
    m_server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    //当有连接事件发生时，可能是断开，也可能是新连接，所以需要判断
    //true表示是为连接，false为断开
    if(!conn->connected()){
        ChatService::getChatService()->clientCloseExcepton(conn);
        //断开则释放对应的资源
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp t)
{
    //读取缓冲区的内容，返回string类型
    std::string str = buf->retrieveAllAsString();
    //数据反序列化
    json js = json::parse(str);
    //获取对应消息的的消息处理函数，实现业务和网络分离
    auto handler = ChatService::getChatService()->getHandler(js["msgid"].get<int>());
    //执行处理函数
    handler(conn, js, t);
}