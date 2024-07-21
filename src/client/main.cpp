#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};


// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int);
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char **argv)
{
    //用户输入参数少于3个则有问题
    if (argc < 3)
    {
        cerr << "command invalid! example: ./client 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 创建服务器地址结构
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 客户端和服务器进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    sem_init(&rwsem, 0, 0);

    // 连接服务器成功，启动接收子线程
    std::thread readTask(readTaskHandler, clientfd); 
    //设置子线程分离属性
    readTask.detach();                               

    // main线程用于接收用户输入，负责发送数据
    while(true)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. 登录" << endl;
        cout << "2. 注册" << endl;
        cout << "3. 退出" << endl;
        cout << "========================" << endl;
        cout << "请选择:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // 登录业务
        {
            int id = 0;
            //存储密码
            char pwd[50] = {0};
            cout << "请输入用户ID: ";
            cin >> id;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "请输入密码: ";
            cin.getline(pwd, 50);

            //封装用户输入的消息为json字符串
            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoginSuccess = false;
            //发送消息
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg error:" << request << endl;
            }
            // 等待信号量，由子线程处理完登录的响应消息后，唤醒主线程
            sem_wait(&rwsem); 
                
            if (g_isLoginSuccess)//登录成功则进入聊天菜单
            {
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        }
        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "请输入用户名: ";
            cin.getline(name, 50);
            cout << "请输密码: ";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            //对用户发送的消息序列化
            string request = js.dump();
            //发送消息
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            //客户端向服务器发送消息后，会阻塞在这。
            //等待服务器处理客户端发送的消息，并返回响应消息。
            //然后等待子线程处理完响应消息，最后唤醒主线程
            sem_wait(&rwsem); 
        }
        break;
        case 3: // 退出业务
            //释放资源
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        default:
            cerr << "command invalid! example: 1!" << endl;
            break;
        }
    }

    return 0;
}

// 处理注册的响应逻辑
/**
 * @brief 解析服务器发送的消息，判断注册是否有误
 * @param[in] responsejs 服务器返回的响应json
 */
void doRegResponse(json &responsejs)
{
    if (0 != responsejs["errno"].get<int>()) // 注册失败
    {
        cerr << "用户名已存在, 注册错误，请重试!" << endl;
    }
    else // 注册成功
    {
        cout << "注册成功, 您的用户ID是" << responsejs["id"] << ", 请记住!" << endl;
    }
}

/**
 * @brief 处理登录的响应逻辑,登录成功，初始化全局变量
 * @param[in] responsejs 服务器返回的响应json
 */
void doLoginResponse(json &responsejs)
{
    if (0 != responsejs["errno"].get<int>()) // 登录失败
    {
        //将登录失败的消息返回给客户端
        cerr << responsejs["errmsg"] << endl;
        //更新前客户端登录状态
        g_isLoginSuccess = false;
    }
    else // 登录成功
    {
        // 记录当前用户的id和name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends"))//判断json是否有friends这个字段
        {
            // 初始化
            g_currentUserFriendList.clear();
            //json字符串隐式转换为vector
            vector<string> vec = responsejs["friends"];
            for (string &str : vec)
            {
                //反序列化
                json js = json::parse(str);
                //封装用户信息
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                //存储到全局变量
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups"))//判断json是否有groups这个字段
        {
            // 初始化
            g_currentUserGroupList.clear();
            
            //json字符串隐式转换为vector
            vector<string> vec1 = responsejs["groups"];
            for (string &groupstr : vec1)
            {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setGroupName(grpjs["groupname"]);
                group.setGroupDesc(grpjs["groupdesc"]);

                vector<string> vec2 = grpjs["users"];
                for (string &userstr : vec2)
                {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setGroupRole(js["role"]);
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线消息  个人聊天信息或者群组消息
        if (responsejs.contains("offlinemsg"))//判断json是否有offlinemsg这个字段
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                //反序列化
                json js = json::parse(str);
                //消息格式：时间 [发送者id]发送者名称 said: 消息内容
                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    //消息格式：群消息[群组id]: 时间 [发送者id]发送者名称 said: 消息内容
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" 
                         << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
        //更新客户端登录状态
        g_isLoginSuccess = true;
    }
}

/**
 * @brief 子线程 - 接收线程,负责处理服务器发送的消息
 * @param[in] 客户端socket
 */
void readTaskHandler(int clientfd)
{
    while(true)
    {
        //读取缓冲区
        char buffer[1024] = {0};
        //阻塞读取服务端发送的消息
        int len = recv(clientfd, buffer, 1024, 0); 
        if (-1 == len || 0 == len)
        {
            //服务器与客户端连接发送异常
            close(clientfd);
            exit(-1);
        }

        // 接收服务器转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        //msgtype消息类型
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype)//单人聊天类型
        {
            //消息格式：时间 [发送者id]发送者名称 said: 消息内容
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype)//群组聊天
        {
            //消息格式：群消息[群组id]:时间 [发送者id]发送者名称 said: 消息内容
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" 
                 << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype)//登录消息
        {
            doLoginResponse(js); // 处理登录响应的业务逻辑
            sem_post(&rwsem);    // 通知主线程，登录结果处理完成
            continue;
        }

        if (REG_MSG_ACK == msgtype)//注册消息
        {
            doRegResponse(js);
            sem_post(&rwsem);    // 通知主线程，注册结果处理完成
            continue;
        }
    }
}

/**
 * @brief 显示当前登录成功用户的基本信息
 */
void showCurrentUserData()
{
    cout << "======================欢迎登录======================" << endl;
    cout << "当前登录用户 => ID:" << g_currentUser.getId() << " 名称:" << g_currentUser.getName() << endl;
    cout << "----------------------好友列表---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
            cout << "ID: " << user.getId() << " 名称: " << user.getName() << " 状态: " << user.getState() << endl;
    }
    cout << "----------------------群组列表----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << "群ID: " << group.getId() << " 群名称: " << group.getGroupName() << " 群描述: " 
                 << group.getGroupDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << "\t" << "ID: " << user.getId() << " 名称: " << user.getName() << " 状态: " << user.getState() 
                     << " 角色:" << user.getGroupRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

void help(int fd = 0, string str = "");

void chat(int, string);

void addfriend(int, string);

void creategroup(int, string);

void addgroup(int, string);

void groupchat(int, string);

void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help       ", "显示所有指令 格式 help"},
    {"chat       ", "一对一的聊天 格式 chat:friendid:message"},
    {"addfriend  ", "添加新的好友 格式 addfriend:friendid"},
    {"creategroup", "创建聊天群组 格式 creategroup:groupname:groupdesc"},
    {"addgroup   ", "加入聊天群组 格式 addgroup:groupid"},
    {"groupchat  ", "向群发送消息 格式 groupchat:groupid:message"},
    {"loginout   ", "注销当前用户 格式 loginout"}};


// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 主聊天页面程序
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        //获取一行用户输入的的指令
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        //查找“:”
        int idx = commandbuf.find(":");
        if (-1 == idx)//没找到
        {
            ////可能这个指令没有参数
            command = commandbuf;
        }
        else
        {
            //截取指令的名称部分的字符串
            command = commandbuf.substr(0, idx);
        }
        //查找是否存在改指令
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "没有该指令！" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

/**
 * @brief 显示客户端的命令,指令格式: help
 */
void help(int, string)
{
    cout << "客户端指令列表 >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

/**
 * @brief 添加好友回调，指令格式: addfriend:friendid
 * @param[in] clientfd 客户端socket
 * @param[in] str 指令参数
 */
void addfriend(int clientfd, string str)
{
    //获取指令参数
    int friendid = atoi(str.c_str());
    //序列化
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    //发送消息
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

/**
 * @brief 一对一聊天，指令格式: chat:friendid:message
 * @param[in] clientfd 客户端socket
 * @param[in] str 指令参数
 */
void chat(int clientfd, string str)
{
    //对指令参数解析
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    //截取好友id
    int friendid = atoi(str.substr(0, idx).c_str());
    //截取发送的消息内容
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

/**
 * @brief 创建群组，指令格式: creategroup:groupname:groupdesc
 * @param[in] clientfd 客户端socket
 * @param[in] str 指令参数
 */
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }
    //截取群组名称
    string groupname = str.substr(0, idx);
    //截取群组描述
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

/**
 * @brief 加入群组，指令格式: addgroup:groupid
 * @param[in] clientfd 客户端socket
 * @param[in] str 指令参数
 */
void addgroup(int clientfd, string str)
{
    //获取群组id
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

/**
 * @brief 群组聊天，指令格式: groupchat:groupid:message
 * @param[in] clientfd 客户端socket
 * @param[in] str 指令参数
 */
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    //截取群组id
    int groupid = atoi(str.substr(0, idx).c_str());
    //截取消息内容
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}


/**
 * @brief 注销，指令格式: loginout
 */
void loginout(int clientfd, string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    //获取当前登录的用户的id
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)//发送失败
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
        //退出登录更新集合
        g_currentUserFriendList.clear();
        g_currentUserGroupList.clear();

    }   
}

/**
 * @brief 获取系统时间（聊天信息需要添加时间信息）
 */
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}