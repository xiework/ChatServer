#ifndef PUBLIC_H
#define PUBLIC_H

//这个头文件是server和client共同使用的头文件

//消息类枚举
enum MsgType{
    LOGIN_MSG = 1,      //登录消息类型
    LOGIN_MSG_ACK,      //登录响应消息 
    LOGINOUT_MSG,       //注销消息类型
    REG_MSG,            //注册消息类型
    REG_MSG_ACK,        //注册响应消息
    ONE_CHAT_MSG,       //单人聊天消息
    ADD_FRIEND_MSG,     //添加好友消息
    CREATE_GROUP_MSG,   //创建群组消息
    ADD_GROUP_MSG,      //加入群主消息
    GROUP_CHAT_MSG      //多人聊天消息
};

#endif