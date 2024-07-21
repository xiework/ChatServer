#include "db.hpp"
#include <muduo/base/Logging.h>

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

MySQL::MySQL()
{
    m_conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (m_conn != nullptr)
        mysql_close(m_conn);
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(m_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(m_conn, "set names gbk");
        LOG_INFO << "connection scuess!";
    }
    else
    {
        LOG_ERROR << "connection fail!";
    }
    return p;
}

bool MySQL::update(string sql)
{
    if (mysql_query(m_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(m_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(m_conn);
}