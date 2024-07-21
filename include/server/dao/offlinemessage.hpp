#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H
#include <string>
//维护离线消息表
class offlineMessage{
public:
    offlineMessage(int id = -1, std::string _messge = "") : userid(id), message(_messge){}

    void setUserid(int _id) {this->userid = _id;}
    void setMessge(std::string _message) {this->message = _message;}
    int getUserid() const {return this->userid;}
    std::string getMessage() const {return this->message;}
private:
    int userid;
    std::string message;
};
#endif