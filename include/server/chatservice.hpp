#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include "usermodel.hpp"
#include <mutex>
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

//声明一个函数变量，用于处理不同的业务
using MsgHandler =std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>; 

//服务器业务处理类
//这个类为单例对象，用于处理消息
class ChatService{
public:
    /**
     * @brief 处理登录业务
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    
    /**
     * @brief 处理注册业务
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    /**
     * @brief 处理一对一聊天
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    /**
     * @brief 添加好友
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

     /**
     * @brief 创建群组
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    /**
     * @brief 加入群组
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

     /**
     * @brief 群组聊天
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    /**
     * @brief 退出登录
     * @param[in] conn 连接对象的指针
     * @param[in] js 消息序列化对象
     * @param[in] time 产生消息的时间
     * @return 无返回值
     */
    void loginOut(const TcpConnectionPtr& conn, json& js, Timestamp time);

    /**
     * @brief 当监听通道有消息时调用的函数，将消息传送给指定的用户
     * @param[in] userid 接收消息的用户id
     * @param[in] msg 消息内容
     * @return 无返回值
     */
    void handlerRedisSubscribeMessage(int userid, std::string msg);


    /**
     * @brief 通过对外的静态接口，提供一个实例化的处理类对象的指针
     * @return 返回一个ChatService的指针
     */
    static ChatService* getChatService();

    /**
     * @brief 获取消息对应的处理函数
     * @param[in] msgid 消息类型
     * @return 返回一个处理函数
     */
    MsgHandler getHandler(int msgid);
    /**
     * @brief 处理异常关闭的用户端连接
     * @param[in] conn 连接到客户端的连接
     * @return 无返回值
     */
    void clientCloseExcepton(const TcpConnectionPtr& conn);

    /**
     * @brief 服务异常重置状态
     * @return 无返回值
     */
    void reset();


    
private:
    //将构造函数私有化
    ChatService();
    //业务处理集合
    //int表示处理的消息的类型，MsgHandler表示对应的处理消息对应的函数
    std::unordered_map<int, MsgHandler> m_MsgHandlerMap; 

    //用于存储每个用户的连接对象，以方便后续操作
    std::unordered_map<int, TcpConnectionPtr> m_userConnMap;

    //互斥锁保证在对m_userConnMap操作时是线程安全
    std::mutex m_mutex;
    // 数据操作类对象
    UserModel m_userModel;
    offlineMessageModel m_offlineMsgModel;
    FriendModel m_friendModel;
    GroupModel m_groupModel;
    Redis redis;
};

#endif