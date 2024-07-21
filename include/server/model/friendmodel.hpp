#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include "friend.hpp"
#include "user.hpp"
#include <vector>
//好友表的相关操作
class FriendModel{
public:
    /**
     * @brief 添加好友
     * @param[in] _friend 添加的好友对象
     * @return 返回是否添加成功
     */
    bool insert(Friend _friend);

    /**
     * @brief 查询好友
     * @param[in] userid 要查询的用户的好友
     * @return 返回该对象的好友信息集合
     */
    std::vector<User> query(int userid);
};
#endif