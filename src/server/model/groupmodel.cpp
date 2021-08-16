#include"groupmodel.hpp"
#include"db.h"

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid,int groupid,string role)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into groupuser(groupid,userid,grouprole) values('%d','%d','%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql,"select b.id,b.groupname,b.groupdesc from groupuser a join allgroup b on a.groupid=b.id where a.userid=%d",userid);
    MySQL mysql;
    vector<Group> groupVec;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }

        for(Group &group : groupVec)
        {
            sprintf(sql,"select b.id,b.name,b.state,a.grouprole from groupuser a join user b on a.userid=b.id where groupid=%d",group.getId());
            res = mysql.query(sql);
            if(res!=nullptr)
            {
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr)
                {
                    GroupUser groupUser;
                    groupUser.setId(atoi(row[0]));
                    groupUser.setName(row[1]);
                    groupUser.setState(row[2]);
                    groupUser.setRole(row[3]);
                    group.getUsers().push_back(groupUser);
                }
                mysql_free_result(res);
            }
        } 
    }
    return groupVec;
}

vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid!=%d",groupid,userid);
    MySQL mysql;
    vector<int> idVec;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}