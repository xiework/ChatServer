#ifndef USERMODEL_H
#define USERMOODEL_H

#include "user.hpp"

//用户表操作类
class UserModel{
public:
    /**
     * @brief 向数据库插入用户
     * @param[in] user 封装了用户信息
     * @return 返回是否插入成功
     */
    bool insert(User& user);
    /**
     * @brief 查询数据库是否存在用户
     * @param[in] id 要查询的用户id
     * @return 返回查询到的用户对象
     */
    User query(int id);

    /**
     * @brief 修改用户状态
     * @param[in] user 要修改的用户对象
     * @return 返回是否修改成功
     */
    bool updateState(User& user);

    /**
     * @brief 重置用户状态
     * @return 返回是否重置成功
     */
    bool resetState();
private:
};
#endif