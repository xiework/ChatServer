#include "redis.hpp"
#include <iostream>
using namespace std;
Redis::Redis() : m_publish_context(nullptr), m_subscribe_context(nullptr)
{

}

Redis::~Redis()
{
    //释放资源
    if (m_publish_context != nullptr)
        redisFree(m_publish_context);
    if (m_subscribe_context != nullptr)
        redisFree(m_subscribe_context);
}
    
bool Redis::connect()
{
    //负责发布消息的上下文连接
    m_publish_context = redisConnect("127.0.0.1",6379);
    if (m_publish_context == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    //负责订阅消息的上下文连接
    m_subscribe_context = redisConnect("127.0.0.1",6379);
    if (m_subscribe_context == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    //当订阅消息时m_subscribe_contex会阻塞
    //所以创建一个线程负责将订阅中的通道上的消息传给上层
    thread t([&]() {observer_channel_message(); });
    t.detach();
    //cout << "connect redis-server success!" << endl;
    return true;
}

bool Redis::publish(int channel,string message)
{
    //在m_publish_contex连接中执行redis指令，发送消息
    /*
        redisCommand这个函数回调用三个函数
        如下
        1.首先，调用redisAppendCommand函数将命令发送到redis服务器的缓存中
        2.然后，调用redisBufferWrite函数将命令发送到redis服务器中，让redis服务器执行命令
        3.最后，调用redisGetReply函数阻塞等待redis服务器返回结果
    */
    redisReply* reply = (redisReply*) redisCommand(m_publish_context, "publish %d %s",channel, message.c_str());
    if(reply == nullptr)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    //cout << "publish success!" << endl;
    return true;
}

bool Redis::subscribe(int channel)
{
    //由于订阅消息是会阻塞线程，等待服务器返回，所以不能调用redisCommand函数，需要将等待服务器返回的工作交给一个线程来完成
    //所以这里只做订阅通道，不接受消息
    if (REDIS_ERR == redisAppendCommand(this->m_subscribe_context, "subscribe %d",channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        //循环发送缓冲内容，发送完毕后，done会变为1
        if (REDIS_ERR == redisBufferWrite(this->m_subscribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    //cout << "subscribe success! -> "<< channel << endl;
    return true;
}

bool Redis::unsubscribe(int channel)
{
    //unsubscribe同理
    if (REDIS_ERR == redisAppendCommand(this->m_subscribe_context, "unsubscribe %d",channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        //循环发送缓冲内容，发送完毕后，done会变为1
        if (REDIS_ERR == redisBufferWrite(this->m_subscribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    //cout << "unsubscribe success!" << endl;
    return true;
}

void Redis::observer_channel_message()
{
    //cout << "thread start" << endl;
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->m_subscribe_context,(void**)&reply))
    {
        //订阅收到的消息是一个带三元数的数据
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //element[1]通道标识，element[2]消息内容
            m_notify_memssage_handler(atoi(reply->element[1]->str), reply->element[2]->str);
            cout << "m_notify_memssage_handler success!" << endl;
        }
        freeReplyObject(reply);
    }
    cerr << ">>>>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->m_notify_memssage_handler = fn;
    cout << "handler ok" << endl;
}