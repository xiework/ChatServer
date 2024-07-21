#ifndef GROUP_H
#define GRPUP_H
#include "groupuser.hpp"
#include <string>
#include <vector>

//allgroup表
class Group{
public:
    Group(int _id = -1, std::string name = "", std::string desc = "") : id(_id), groupname(name), groupdesc(desc) {}
    void setId(int _id) {this->id = _id;}
    void setGroupName(std::string name) {this->groupname = name;}
    void setGroupDesc(std::string desc) {this->groupdesc = desc;}
    
    int getId() const {return this->id;}
    std::string getGroupName() const {return this->groupname;}
    std::string getGroupDesc() const {return this->groupdesc;}
    std::vector<GroupUser>& getUsers() {return this->groupusers;}
 
private:
    //标识一条记录
    int id;
    //组的名称
    std::string groupname;
    //组的描述
    std::string groupdesc;
    //组内的用户
    std::vector<GroupUser> groupusers; 
};
#endif