#ifndef GROUPUSER_H
#define GROUPUSER_H
#include <user.hpp>
#include <string>

//群组用户，继承用户。群组用户类多了一个角色属性，用于表明该用户在某个群组中的角色
class GroupUser : public User{
public:
    void setGroupRole(std::string role) {this->grouprole = role;}

    std::string getGroupRole() const {return this->grouprole;}

private:
    //组内用户角色
    std::string grouprole;
};
#endif