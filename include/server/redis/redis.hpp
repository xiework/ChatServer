#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

class Redis{
public:
    /**
     * @brief 资源初始化
     */
    Redis();
    /**
     * @brief 释放资源
     */
    ~Redis();

    /**
     * @brief 连接redis服务器
     * @return 返回是否连接成功
     */
    bool connect();

    /**
     * @brief 向指定的通道发送消息
     * @param[in] channel 标识一个通道
     * @param[in] message 消息内容
     * @return 返回是否发送成功
     */
    bool publish(int channel,string message);

    /**
     * @brief 订阅指定的通道，等待该通道的消息
     * @param[in] channel 标识一个通道
     * @return 返回是否接收成功
     */
    bool subscribe(int channel);

    /**
     * @breif 取消订阅一个通道。当用户下线，不再监听该通道
     * @param[in] channel 标识一个通道
     * @return 返回是否取消成功
     */
    bool unsubscribe(int channel);

    /**
     * @brief 接收订阅的通道中的消息
     */
    void observer_channel_message();

    /**
     * @brief 初始化回调函数，当订阅的通道到消息时，就会触发该回调函数，向上层传输消息
     * @param[in] fn 回调的函数的指针
     */
    void init_notify_handler(function<void(int,string)> fn);

private:
    //上下文对象，负责publish消息（发布消息）
    redisContext* m_publish_context;
    //上下文对象，负责subcribe消息（订阅消息）
    redisContext* m_subscribe_context;
    //回调函数，收到订阅后，将消息发送给service
    function<void(int,string)> m_notify_memssage_handler;

};

#endif