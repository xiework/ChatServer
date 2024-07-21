#include "friendmodel.hpp"
#include "db.hpp"

bool FriendModel::insert(Friend _friend)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userid,friendid) values(%d,%d)",
         _friend.getUserId(), _friend.getFriendId());
    MySQL mysql;
    if (mysql.connect()){
        return mysql.update(sql);
    }
    return false;
}


std::vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    //联合查询获取好友的信息
    sprintf(sql,"select id,name,state from user join friend on friendid = id where userid=%d",userid);
    //存储好友对象
    vector<User> vec;
    MySQL mysql;
    if (mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        if (res != nullptr){
            while ((row = mysql_fetch_row(res)) != nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
        
    }
    return vec;
}