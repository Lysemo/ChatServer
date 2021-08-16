#include"json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>

using namespace std;

//json序列化1
string func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "what are you doing?";

    string strBuf = js.dump();  //json数据对象 --(序列化)--> json字符串
    cout << strBuf << endl;
    return strBuf;
}

//json序列化2
void func2()
{
    json js;
    js["id"] = {1,2,3,4};
    js["msg"]["zhang san"] = "0102";
    js["msg"]["li si"] = "1010";

    cout << js << endl;
}

//json序列化3
void func3()
{
    json js;
    vector<int> arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    map<int,string> mp;
    mp.insert({1,"01"});
    mp.insert({2,"02"});

    js["list"] = arr;
    js["map"] = mp;

    cout << js << endl;
}

int main()
{
    string recvBuf = func1();
    func2();
    func3();

    //数据的反序列化   从json字符串 反序列化成 json数据对象（方便访问）
    json jsBuf = json::parse(recvBuf);
    cout << jsBuf["msg_type"] << endl;
    cout << jsBuf["from"] << endl;
    cout << jsBuf["to"] << endl;
    cout << jsBuf["msg"] << endl;
    return 0;
}