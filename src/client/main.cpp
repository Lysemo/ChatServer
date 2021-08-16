#include<iostream>
#include<vector>
#include"json.hpp"
#include<string>
#include<ctime>
#include<chrono>
#include<thread>
#include<unordered_map>

using namespace std;
using json = nlohmann::json;

#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include"user.hpp"
#include"group.hpp"
#include"public.hpp"

User g_currentUser;

vector<User> g_currentUserFriendList;

vector<Group> g_crrentUserGroupList;

bool isMainMenuRunning = false;

void showCurrentUserData();

void readTaskHandler(int clientfd);

string getCurrentTime();

void mainMenu(int clientfd);

void help(int clientfd = 0, string command = "");

void chat(int clientfd, string command);

void addfriend(int clientfd, string command);

void creategroup(int clientfd, string command);

void addgroup(int clientfd, string command);

void groupchat(int clientfd, string command);

void loginout(int clientfd, string command);

unordered_map<string,string> commandMap = {
    {"help","show all supported command,format:help"},
    {"chat","chat one by one,format:chat:friendid:message"},
    {"addfriend","add friend,format:addfriend:friendid"},
    {"creategroup","create group,format:creategroup:groupname:groupdesc"},
    {"addgroup","add group,format:addgroup:groupid"},
    {"groupchat","chat by group members,format:groupchat:groupid:message"},
    {"loginout","login out,format:loginout"}
};

unordered_map<string,function<void(int,string)>> commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};

int main(int argc,char **argv)
{
    if(argc<3)
    {
        cout << "command invalid, eg: ./ChatCLient 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd)
    {
        cerr << "socker create error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(-1==connect(clientfd,(sockaddr *)&server,sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    for(;;)
    {
        cout << "============================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "============================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get();

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin>>id;
            cin.get();
            cout << "userpassword:";
            cin.getline(pwd,50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
            if(len==-1)
            {
                cerr << "send login msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd,buffer,1024,0);
                if(-1==len)
                {
                    cerr << "recv login response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if(0!=responsejs["errno"].get<int>())
                    {
                        cerr << responsejs["errmsg"] << endl;
                    }
                    else
                    {
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);

                        if(responsejs.contains("friends"))
                        {
                            g_currentUserFriendList.clear();

                            vector<string> vec = responsejs["friends"];
                            for(string &str:vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }

                        if(responsejs.contains("groups"))
                        {
                            g_crrentUserGroupList.clear();

                            vector<string> vec1 = responsejs["groups"];
                            for(string &groupstr:vec1)
                            {
                                json grpjs = json::parse(groupstr);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["groupname"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for(string &userstr:vec2)
                                {
                                    GroupUser user;
                                    json js = json::parse(userstr);
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);
                                    group.getUsers().push_back(user);
                                }
                                g_crrentUserGroupList.push_back(group);
                            }
                        }

                        showCurrentUserData();

                        if(responsejs.contains("offlinemsg"))
                        {
                            vector<string> vec = responsejs["offlinemeg"];
                            for(string &str:vec)
                            {
                                json js = json::parse(str);
                                if(ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    cout << js["time"] << "[" << js["id"] << "]" << js["name"] << "said:" << js["msg"] << endl;
                                }
                                else
                                {
                                    cout << "group msg[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                                }
                            }
                        }
                        static int readthreadnumber = 0;
                        if(readthreadnumber == 0)
                        {
                            std::thread readTask(readTaskHandler,clientfd);
                            readTask.detach();
                            readthreadnumber++;
                        }
                        
                        isMainMenuRunning = true;

                        mainMenu(clientfd);
                    }
                    
                }
                
            }
            

            break;
        }
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:" ;
            cin.getline(name,50);
            cout << "userpassword:";
            cin.getline(pwd,50); 

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
            if(len==-1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd,buffer,1024,0);
                if(len==-1)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if(0!=responsejs["errno"].get<int>())
                    {
                        cerr << name << "is already exist, register error!" << endl;
                    }
                    else
                    {
                        cout << name << " register success, userid is " << responsejs["id"]
                            << ", do not forget it!" << endl;
                    }
                    
                }
                
            }
            
            break;
        }
        case 3:
        {
            close(clientfd);
            exit(0);
            break;
        }
        default:
        {
            cerr << "invalid input!" << endl;
            break;
        }
        }
    }

    return 0;
}

void showCurrentUserData()
{
    cout << "======================login user=================="<<endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "-----------------------friend list----------------------"<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(auto &user:g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list---------------------" << endl;
    if(!g_crrentUserGroupList.empty())
    {
        for(auto &group : g_crrentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for(GroupUser &user:group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "===============================================" << endl;
}


void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if(-1==len || len==0)
        {
            close(clientfd);
            exit(-1);
        }

        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if(ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue; 
        }
        if(GROUP_CHAT_MSG == msgtype)
        {
            cout << "group msg[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue; 
        }
    }
}
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if(-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx);
        }
        auto it = commandHandlerMap.find(command);
        if(it==commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
    }
}

string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year+1900,(int)ptm->tm_mon+1,(int)ptm->tm_mday,
            (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return std::string(date);
}

void help(int,string)
{
    cout << "show command list>>>" << endl;
    for(auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void chat(int clientfd, string command)
{
    int idx = command.find(":");
    if(-1==idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    int friendid = atoi((command.substr(0,idx)).c_str());
    string message = command.substr(idx+1,command.size()-idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send chat msg error ->" << buffer << endl;
    }
}

void addfriend(int clientfd, string command)
{
    int friendid = atoi(command.c_str());

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["friendid"] = friendid;
    js["id"] = g_currentUser.getId();

    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send addfriend msg error ->" << buffer << endl;
    }
}

void creategroup(int clientfd, string command)
{
    int idx = command.find(":");
    if(-1==idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }
    string groupname = command.substr(0,idx);
    string groupdesc = command.substr(idx+1,command.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send creategroup msg error ->" << buffer << endl;
    }
}

void addgroup(int clientfd, string command)
{
    int groupid = atoi(command.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["groupid"] = groupid;
    js["id"] = g_currentUser.getId();

    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send addgroup msg error ->" << buffer << endl;
    }
}

void groupchat(int clientfd, string command)
{
    int idx = command.find(":");
    if(-1==idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    int groupid = atoi(command.substr(0,idx).c_str());
    string message = command.substr(idx+1,command.size()-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send groupchat msg error ->" << buffer << endl;
    }
}

void loginout(int clientfd, string command)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();

    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len)
    {
        cerr << "send loginout msg error ->" << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
    
}