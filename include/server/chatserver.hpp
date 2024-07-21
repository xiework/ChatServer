#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer{
public:
    /**
     * @brief 初始化ChatServer对象
     * @param[in] loop 事件循环，监听事件
     * @param[in] listenAddr 服务器地址结构 IP + Port
     * @param[in] name 服务器名称（连接的名称）
     * @return 返回ChatServer对象
     */
    ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& name);
    /**
     * @param 启动循环事件
     * @return 无返回值
     */
    void start();

private:
    /**
     * @brief 专门处理连接和断开的事件
     * @param[in] conn 连接的对象的指针
     * @return 无返回值
     */
    void onConnection(const TcpConnectionPtr& conn);

    /**
     * @brief 专门处理读写事件
     * @param[in] conn 连接对象
     * @param[in] buf 读写缓冲区
     * @param[in] t 收到数据的时间按信息
     * @return 无返回值
     */
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp t);
private:
    TcpServer m_server;
    EventLoop* m_loop;
};

#endif