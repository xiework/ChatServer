#include "groupmodel.hpp"
#include "db.hpp"


bool GroupModel::createGroup(Group& group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s','%s')",
        group.getGroupName().c_str(), group.getGroupDesc().c_str());
    MySQL mysql;
    if (mysql.connect()){
        if (mysql.update(sql)){
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}


bool GroupModel::addGroup(int userid, int groupid, std::string role)
{
     char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d,%d,'%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if (mysql.connect()){
       return mysql.update(sql);
    }
    return false;
}


std::vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select b.id,b.groupname,b.groupdesc from groupuser as a join allgroup as b on a.groupid = b.id \
        where userid = %d",userid);

    //查询该用户的群组
    std::vector<Group> groupvec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        if (res != nullptr){
            while ((row = mysql_fetch_row(res)) != nullptr){
                groupvec.push_back(Group(atoi(row[0]), row[1], row[2]));
            }
            mysql_free_result(res);
        }
    }
    //补充group的gruopuser属性
    for(Group& group : groupvec){
        //根据群组id查询到这个组员的信息
        sprintf(sql, "select b.id,b.name,b.state,a.grouprole from groupuser as a join user as b on a.userid = b.id \
            where a.groupid=%d",group.getId());
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        if (res != nullptr){
            while ((row = mysql_fetch_row(res)) != nullptr){
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setGroupRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupvec;
}


std::vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d",groupid);
    vector<int> vec;
    MySQL mysql;
    if (mysql.connect()){
        //查询到指定群组的记录，从中取对应的用户id
        //将对应的用户id保存到集合中（自己除外）
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW  row;
        if (res != nullptr){
            while ((row = mysql_fetch_row(res)) != nullptr){
                if (atoi(row[0]) != userid)
                    vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}