#include "chatservice.hpp"
#include <functional>
#include <public.hpp>
#include <iostream>
#include <muduo/base/Logging.h>
#include "public.hpp"
#include "user.hpp"
#include "offlinemessage.hpp"

using namespace std;
using namespace std::placeholders;
using namespace muduo;
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = m_userModel.query(id);
    if (user.getId() != -1 && user.getPassword() == pwd){//用户存在且密码正确
        //返回消息的json对象
        json response;
        //封装消息类型
        response["msgid"] = MsgType::LOGIN_MSG_ACK;
        if (user.getState() == "online"){//账号密码都对得上，但是已经登录
            response["errno"] = 2;
            response["errmsg"] = "用户已登录，不可重复登录";
        }else{
            
            {
                //加锁操作,局部变量，退出作用域自动解锁
                lock_guard<mutex> lock(m_mutex);
                //将登录的用户添加的已登录用户集合
                m_userConnMap.insert({id,conn});
            }
            //订阅消息
            redis.subscribe(id);
            //更新用户登录状态
            user.setState("online");
            m_userModel.updateState(user);
            //封装是否有错误
            response["errno"] = 0;
            //封装插入的用户id
            response["id"] = user.getId();
            //封装用户名
            response["name"] = user.getName();
            
            //获取改用户的离线消息
            vector<std::string> vec = m_offlineMsgModel.query(id);
            if (!vec.empty()){//判断是否有离线消息
                //对离线消息集合序列化
                response["offlinemsg"] = vec;
                //删除该用户的离线消息
                m_offlineMsgModel.remove(id);
            }
            //获取用户的好友信息
            vector<User> vecUser = m_friendModel.query(id);
            if (!vecUser.empty()){
                //将用户对象序列化
                vector<std::string> vecStrUser;
                for (auto user : vecUser){
                    json jss;
                    jss["id"] = user.getId();
                    jss["name"] = user.getName();
                    jss["state"] = user.getState();
                    vecStrUser.push_back(jss.dump());
                } 
                response["friends"] = vecStrUser;
            }
            // 查询用户的群组信息
            vector<Group> vecGroup = m_groupModel.queryGroup(id);
            if (!vecGroup.empty()){
                vector<string> vecGroupStr;
                for (Group group : vecGroup){
                    json groupjson;
                    groupjson["id"] = group.getId();
                    groupjson["groupname"] = group.getGroupName();
                    groupjson["groupdesc"] = group.getGroupDesc();
                    vector<string> groupuser;
                    for (auto user:group.getUsers()){
                        json userjson;
                        userjson["id"] = user.getId();
                        userjson["name"] = user.getName();
                        userjson["state"] = user.getState();
                        userjson["role"] = user.getGroupRole();
                        groupuser.push_back(userjson.dump());
                    }
                    groupjson["users"] = groupuser;
                    vecGroupStr.push_back(groupjson.dump());
                }
                response["groups"] = vecGroupStr;
            }
        } 
        //给客户端发送消息
        conn->send(response.dump());
    }else{
        //用户不存在，或密码不正确
        json response;
        response["msgid"] = MsgType::LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
    

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    //获取用户发送的用户名密码
    string name = js["name"];
    string password = js["password"];
    //将在这些信息封装到用户对象中
    User user;
    user.setName(name);
    user.setPassword(password);
    //将用户插入到数据库中
    bool state = m_userModel.insert(user);
    if (state){//插入成功
        //返回消息的json对象
        json response;
        //封装消息类型
        response["msgid"] = MsgType::REG_MSG_ACK;
        //封装是否有错误
        response["errno"] = 0;
        //封装插入的用户id
        response["id"] = user.getId();
        //给客户端发送消息
        conn->send(response.dump());
    }else{
        json response;
        response["msgid"] = MsgType::REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
    
}


void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    //1.获取用户A发送个哪个用户,假设为用户B
    int toid = js["toid"].get<int>();
    //2.判断用户B是否在线
    {
        //由于要操作m_userConnMap，所以要加锁
        lock_guard<mutex> lock(m_mutex);
        auto it = m_userConnMap.find(toid);
        //为什么发送消息要加锁
        //原因：如果不加锁，在发消息前，检查到用户B在线，但当准备发送消息的时候，用户B下线了，那么发送的消息就会无人接收而消失
        if (it != m_userConnMap.end()){//找到表示用户B在线
            //将用户A发送的消息转发给用户B,所以要用到用户B与服务器连接的连接对象
            it->second->send(js.dump());
            return;
        }
    }
    //查询用户B是否在线
    User user = m_userModel.query(toid);
    if (user.getState() == "online")
    {
        //发布消息
        redis.publish(toid,js.dump());
        std::cout << "send redis" << endl;
        return;
    }

    //没有找到表示用户下线，则将消息存储到数据库中，当用户B上线后，转发给其
    offlineMessage offmsg(toid,js.dump());
    m_offlineMsgModel.insert(offmsg);
}


void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //添加好友
    m_friendModel.insert(Friend(userid,friendid));
}




void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    //创建群组的用户
    int userid = js["id"].get<int>();
    //群组的名称
    std::string name = js["groupname"];
    //群组的描述
    std::string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if(m_groupModel.createGroup(group)){
        //群组创建成功，将创始人加入到群组中
        m_groupModel.addGroup(userid,group.getId(),"creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    m_groupModel.addGroup(userid,groupid,"normal");

}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> otherUsers = m_groupModel.queryGroupUsers(userid,groupid);
    //给每个组员发送消息
    lock_guard<mutex> lock(m_mutex);
    for (int user : otherUsers){
        auto it = m_userConnMap.find(user);
        if (it != m_userConnMap.end()){//如果组员在线直接转发消息
            it->second->send(js.dump());
        }else{
            //用户可能不在当前服务器上
            User u = m_userModel.query(user);
            if (u.getState() == "online")
            {
                redis.publish(u.getId(),js.dump());
                return;
            }
            //不在线，存储离线消息
            m_offlineMsgModel.insert(offlineMessage(user,js.dump()));
        }
    }
}

void ChatService::loginOut(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"];
    {
        lock_guard<mutex> lock(mutex);
        auto it = m_userConnMap.find(id);
        if (it != m_userConnMap.end()){
            m_userConnMap.erase(it);
        }
    }
    //取消订阅
    redis.unsubscribe(id);
    //更新用户状态
    User user(id,"","","offline");
    m_userModel.updateState(user);
}

void ChatService::handlerRedisSubscribeMessage(int userid, std::string msg)
{
    //std::cout << "from redis1" << endl;
    //redis将消息转发到用户所在的服务器上后，发现下线了，所以存储为离线消息
    lock_guard<mutex> lock(m_mutex);
    auto it = m_userConnMap.find(userid);
    if (it != m_userConnMap.end()){
        //将消息发送出去
        //std::cout << "from redis" << endl;
        it->second->send(msg);
        return;
    }
    m_offlineMsgModel.insert(offlineMessage(userid,msg));
}

ChatService* ChatService::getChatService()
{
    static ChatService chatservice;
    return &chatservice;
}


MsgHandler ChatService::getHandler(int msgid)
{
    //先判断要找的消息是否有处理函数
    auto it = m_MsgHandlerMap.find(msgid);
    if (it == m_MsgHandlerMap.end()){//如果没有，则返回一个空的处理函数，只负责打印错误日志
        return [=] (const TcpConnectionPtr& conn, json& js, Timestamp time){
            LOG_ERROR << "msgid" << msgid << "can not find handler!";
        };
    }else{
        return it->second;
    }
}

//将业务处理函数与对应的消息类型进行绑定
ChatService::ChatService()
{
    //将业务处理函数与对应的消息类型进行绑定
    m_MsgHandlerMap.insert({MsgType::LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::ONE_CHAT_MSG, std::bind(&ChatService::oneChat,this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::ADD_FRIEND_MSG, std::bind(&ChatService::addFriend,this, _1, _2, _3)});

    m_MsgHandlerMap.insert({MsgType::CREATE_GROUP_MSG, std::bind(&ChatService::createGroup,this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::ADD_GROUP_MSG, std::bind(&ChatService::addGroup,this, _1, _2, _3)});
    m_MsgHandlerMap.insert({MsgType::GROUP_CHAT_MSG, std::bind(&ChatService::groupChat,this, _1, _2, _3)});

    if(redis.connect())
    {
        //绑定回调函数
        redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage,this,_1,_2));
    }
    

}
void ChatService::clientCloseExcepton(const TcpConnectionPtr& conn)
{
    User user;
    {
        lock_guard<mutex> lock(m_mutex);
        //从连接的用户中找到断连的用户
        for (auto it = m_userConnMap.begin(); it != m_userConnMap.end(); it++){
            if (it->second == conn){
                //保存其id
                user.setId(it->first);
                m_userConnMap.erase(it);
                
                break;
            }
        }
    }
    //取消订阅
    redis.unsubscribe(user.getId());
    //更新用户状态
    if (user.getId() != -1){
        //将断连的用户的状态改成offline
        user.setState("offline");
        m_userModel.updateState(user);
    }
}

void ChatService::reset()
{
    //把online状态的的用户，设置为offline
    m_userModel.resetState();
}