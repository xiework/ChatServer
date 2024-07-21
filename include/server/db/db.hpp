#ifndef DB_H
#define DB_H

#include <string>
#include <mysql/mysql.h>
using namespace std;


// 数据库操作类
class MySQL
{
public:
    /**
     * @brief 初始化数据库连接
     * @return 返回一个数据库操作对象
     */
    MySQL();

    /**
     * @brief 释放数据库连接资源
     */
    ~MySQL();


    /**
     * @brief 连接数据库
     * @return 返回是否连接成功，true表示连接成功
     */
    bool connect();
   
    /**
     * @brief 更新操作
     * @param[in] sql 要执行的sql语句
     * @return 返回是否执行成功，true表示执行成功
     */ 
    bool update(string sql);
   
    // 查询操作
    /**
     * @brief 查询操作
     * @param[in] sql 要执行的sql语句
     * @return 返回一个结果集
     */
    MYSQL_RES* query(string sql);

    /**
     * @brief 获取连接对象指针
     * @return 返回一个连接对象指针
     */
    MYSQL* getConnection() {return m_conn;}
private:
    //连接数据库的对象
    MYSQL* m_conn;
};

#endif