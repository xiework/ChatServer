#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include "offlinemessage.hpp"
#include <vector>
#include <string>
//离线消息表相关操作
class offlineMessageModel{
public:
    /**
     * @brief 添加离线消息
     * @param[in] offmsg 要插入的消息对象
     * @return 无返回值
     */
    bool insert(offlineMessage offmsg);
    /**
     * @brief 删除离线消息
     * @param[in] 根据id删除离线消息
     */
    bool remove(int id);

    /**
     * @brief 查询离线消息
     * @param[in] 根据id查询离线消息
     * @return 返回离线对象集合
     */
    std::vector<std::string> query(int id);
};
#endif