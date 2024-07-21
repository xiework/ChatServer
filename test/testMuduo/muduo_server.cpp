#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
#include <functional>
#include <iostream>
using namespace std::placeholders;
/* 基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建事件循环指针eventLoop，事件循环（反应堆）
3.根据TcpServer构造函数写ChatServer构造函数
4.在当前的服务器类的构造函数中，注册处理连接和处理读写事件的回调函数
5.设置合适的线程数量，由muduo库自动分配I/O线程和worker线程
*/
class ChatServer{
public:
    /**
     * @brief 初始化ChatServer对象
     * @param[in] loop 事件循环
     * @param[in] listenAddr 地址结构 IP + Port
     * @param[in] name 服务器名称（连接的名称）
     * @return 返回ChatServer对象
     */
    ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& name)
                :m_server(loop, listenAddr, name), m_eventLoop(loop) {
                    //给服务器注册新用户连接或断开连接的回调
                    m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

                    //给服务器注册用户有读写事件时的回调
                    m_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

                    //设置合适的线程个数, 一个I/O线程，三个worker线程
                    m_server.setThreadNum(4);
                }
    /**
     * @param 启动循环事件
     * @return 无返回值
     */
    void start(){
        m_server.start();
    }
private:
    /**
     * @brief 专门处理连接和断开的事件
     * @param[in] conn 连接的对象的指针
     * @return 无返回值
     */
    void onConnection(const muduo::net::TcpConnectionPtr& conn){
        if (conn->connected()){//当与服务器连接成功
            //打印对端的地质结构和本机的地址结构，以及连接的状态
            std::cout << conn->peerAddress().toIpPort() << "->" 
                << conn->localAddress().toIpPort() << " state: online" << std::endl;
        }else{//没有连接上
            std::cout << conn->peerAddress().toIpPort() << "->" 
                << conn->localAddress().toIpPort() << " state: offline" << std::endl;
            conn->shutdown();//回收客户端的fd, 相当于close(fd)
            //loop->quit()关闭监听，相当于关闭服务器
        }
    }
    /**
     * @brief 专门处理读写事件
     * @param[in] conn 连接对象
     * @param[in] buf 读写缓冲区
     * @param[in] t 收到数据的时间按信息
     * @return 无返回值
     */
    void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp t){
        std::string str = buf->retrieveAllAsString();//读取缓冲区的内容，返回string类型
        //打印信息
        std::cout << "from " << conn->peerAddress().toIpPort() <<" recv data:"
            << str << " time:" << t.toString() << std::endl;
        //将读到的信息返回回去
        conn->send(str);
    }
private:
    muduo::net::TcpServer m_server;
    muduo::net::EventLoop* m_eventLoop;
};

int main() {
    muduo::net::EventLoop loop; //相当于epoll
    muduo::net::InetAddress addr("127.0.0.1", 6000);
    ChatServer cs(&loop, addr, "chat");
    //启动服务器，相当于将fd挂到epoll上，进行一些初始化
    cs.start();
    //循环监听事件，相当于epoll_wait,等待连接或读写事件
    loop.loop();
    return 0;
}