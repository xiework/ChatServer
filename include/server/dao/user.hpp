#ifndef USER_H
#define USER_H
#include <string>
//用户封装类
class User{
public:
    User(int _id = -1, std::string _name = "", std::string _password = "", std::string _state = "offline")
        : id(_id), name(_name), password(_password), state(_state) {}
    
    void setId(int id) {this->id = id;}
    void setName(std::string name) {this->name = name;}
    void setPassword(std::string password) {this->password = password;}
    void setState(std::string state) {this->state = state;}

    int getId() const {return this->id;}
    std::string getName() const {return this->name;}
    std::string getPassword() const {return this->password;}
    std::string getState() const {return this->state;}
protected: 
    int id;
    std::string name;
    std::string password;
    std::string state;
};
#endif