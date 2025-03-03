# ChatServer
基于muduo库实现的集群聊天服务器和客户端源码  
利用nginx tcp模块实现负载均衡以及使用redis的订阅/发布实现等功能跨服务器聊天
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
**bin** -------------- 存放可执行文件，分别是客户端和服务端  
**build** ------------ 存放编译过程中产生的文件（为了上传方便，所以是空目录）   
**include** ---------- 存放头文件  
**src** --------------- 存放源文件  
**test** -------------- 存放测试文件  
**thirdparty** ------ 存放第三方头文件  
**autobuild.sh** ---- 可执行脚本，用于编译项目  
**CMakeLists.txt** -- cmake文件，产出makefile文件  
**picture** ---------- 存放项目运行的截图
## 编译项目
在项目目录下运行autobuild.sh  
运行方式： ./autobuild.sh  
## 运行项目
### 单机环境下运行  
server  服务器可执行文件 例： ./server 127.0.0.1 6000  
client  客户端可执行文件 例： ./client 127.0.0.1 6000 
### 集群环境下运行
在集群环境下运行，需要配置nginx,启动nginx服务器和redis服务器  
在linux系统这个/usr/local/nginx/conf目录下找到nginx.conf配置文件,在该文件中配置tcp负载均衡模块，然后启动nginx  
./server 127.0.0.1 6000  
./client 127.0.0.1 8000  
## 项目运行截图
客户端运行截图
![image](./picture/client1.png)
  
![image](./picture/client2.png)  
服务端运行截图  
在控制台打印运行日志
![image](./picture/server.png) 
