#include "offlinemessagemodel.hpp"
#include "db.hpp"

bool offlineMessageModel::insert(offlineMessage offmsg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage(userid,message) values(%d, '%s')", 
        offmsg.getUserid(), offmsg.getMessage().c_str());
    MySQL mysql;
    if (mysql.connect()){
        return mysql.update(sql);
    }
    return  false;
}

bool offlineMessageModel::remove(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", id);
     MySQL mysql;
    if (mysql.connect()){
        return mysql.update(sql);
    }
    return  false;
}


std::vector<std::string> offlineMessageModel::query(int id)
{
    vector<std::string> vec;
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage");
    MySQL mysql;
    if (mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        if (res != nullptr){
            //不断读取结果集将消息取出
            while ((row = mysql_fetch_row(res)) != nullptr)
                vec.push_back(row[0]);
            mysql_free_result(res);
        }
        
    }
    return vec;
}