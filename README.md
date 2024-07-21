# ChatServer
基于muduo库实现的集群聊天服务器源码和客户端，利用nginx tcp模块实现负载均衡以及使用redis的订阅/发布功能
## 项目功能
用户登录客户端后，可以向其他用户发送消息，可以一对一聊天，同样也可以实现群聊。除此之外还有创建群组，加入群组，添加好友等功能。  
如果对客户端的功能进行扩展也相当容易。
## 开发环境
Ubuntu20.04  
gcc 7.5  
vscode  
mysql 8.0  
redis 6.2.6  
nginx 1.12.2
cmake 3.26.1
muduo
## 目录结构
**bin** 存放可执行文件，分别是客户端和服务端  
**build** 存放编译过程中产生的文件  
**include** 存放头文件  
**src** 存放源文件  
**test** 存放测试文件  
**thirdparty** 存放第三方头文件  
**autobuild.sh** 可执行脚本，用于编译项目  
**CMakeLists.txt** cmake文件，产出makefile文件  
