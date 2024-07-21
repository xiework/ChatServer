#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"

//维护群组信息操作
class GroupModel{
public:
    /**
     * @brief 创建群组
     * @param[in] group 群组对象
     * @return 返回是否创建成功
     */
    bool createGroup(Group& group);

    /**
     * @brief 加入群组
     * @param[in] userid 用户id
     * @param[in] groupid 要加入的群组的id
     * @param[in] role 在群组中的角色
     * @return 返回是是否加入成功
     */
    bool addGroup(int userid, int groupid, std::string role);

    /**
     * @brief 查询用户所在的群组
     * @param[in] userid 要查询的用户
     * @return 返回该用户所在的群组
     */
    std::vector<Group> queryGroup(int userid);

    /**
     * @brief 向指定群组发送消息
     * @param[in] userid 要发送消息的用户
     * @param[in] groupid 指定在哪个群组发送消息
     * @return 返回要接收消息的用户集合，集合中存放用户的id
     */
    std::vector<int> queryGroupUsers(int userid,int groupid);
};
#endif