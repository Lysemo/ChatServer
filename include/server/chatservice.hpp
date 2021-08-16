#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<functional>
#include"offlinemsgmodel.hpp"
#include"friendmodel.hpp"
#include"groupmodel.hpp"
#include"redis.hpp"

#include<mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include"json.hpp"
using json = nlohmann::json;

#include"usermodel.hpp"

using MsgHandler = std::function<void(const TcpConnectionPtr &,json &,Timestamp)>;

class ChatService
{
public:

    static ChatService* instance();

    void reset();

    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time);

    bool createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp time);

    void clientCloseException(const TcpConnectionPtr&);

    void handlerRedisSubscribeMessage(int userid,string msg);
    
    MsgHandler getHandler(int msgid);

private:
    ChatService();
    unordered_map<int,MsgHandler> _msgHandlerMap;

    unordered_map<int,TcpConnectionPtr> _userConnMap;

    mutex _connMutex;

    UserModel _userModel;

    OfflineMsgModel _offlineMsgModel;

    FriendModel _friendModel;

    GroupModel _groupModel;

    Redis _redis;
};

#endif