#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include"group.hpp"

class GroupModel
{
public:
    bool createGroup(Group &group);

    void addGroup(int userid,int groupid,string role);

    vector<Group> queryGroups(int userid);

    vector<int> queryGroupUsers(int userid,int groupid);

};

#endif