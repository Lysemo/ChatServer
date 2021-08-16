# ChatServer
基于muduo网络库、nginx tcp负载均衡、发布-订阅Redis中间件的集群聊天服务器及其客户端
## 目录体系
- bin : 生成的可执行文件
- build : 编译产生的中间文件
- include : server和client的头文件
- src : server和client的源代码
- test : muduo库和json库的测试代码
- thirdparty : 第三方库，eg.json.hpp
- CMakeLists.txt : CMake配置文件
- autobuild.sh : 自动化编译脚本 
## 环境
- redis
- mysql
- nginx
- muduo
- json
