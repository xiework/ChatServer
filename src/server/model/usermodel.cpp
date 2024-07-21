#include "usermodel.hpp"
#include "db.hpp"
bool UserModel::insert(User& user)
{
    //1.组装mysql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')", 
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    //2.执行sql语句
    MySQL mysql;
    if (mysql.connect()){//连接数据库
        if (mysql.update(sql)){//执行sql语句
            //得到插入成功的数据的userid
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id=%d", id);
    //2.执行sql语句
    MySQL mysql;
    if (mysql.connect()){//连接数据库
        //执行查询语句将结果放到res中
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr){
            //从res取出一行数据
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr){
                //利用取出的一个行数据封装一个user对象
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                return user;
            }
            mysql_free_result(res);
        }
        
    }
    return User();
}

bool UserModel::updateState(User& user)
{
    //1.写sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d",user.getState().c_str(),user.getId());
    //2.执行sql语句
    MySQL mysql;
    if (mysql.connect()){
        if (mysql.update(sql))
            return true;
    }
    return false;
}

bool UserModel::resetState()
{
    //1.写sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    //2.执行sql语句
    MySQL mysql;
    if (mysql.connect()){
        if (mysql.update(sql))
            return true;
    }
    return false;
}