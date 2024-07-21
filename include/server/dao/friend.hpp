#ifndef FRIEND_H
#define FRIEND_H

//好友类维护好友表
class Friend{
public:
    Friend(int _id = -1, int _friendid = -1) : userid(_id), friendid(_friendid){}
    void setUserId(int _id) {this->userid = _id;}
    void setFriendId(int _friendid) {this->friendid = _friendid;}
    int getUserId() const {return this->userid;}
    int getFriendId() const {return this->friendid;}
private:
    int userid;
    int friendid;
};
#endif