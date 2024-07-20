#pragma once
#include"google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<google/protobuf/descriptor.h>
#include<muduo/net/TcpConnection.h>
#include<string>
#include<cstring>
#include<functional>
#include<unordered_map>

//service服务类型信息   例如UserService服务类  里面有基于框架重写的Login方法
struct ServiceInfo{
    google::protobuf::Service *m_service;//保存服务对象
    std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务方法
};

//框架提供专门的服务发布rpc服务的网络对象类
class RpcProvider{
public:
    //框架给外部使用，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点,开始提供rpc远程网络调用服务
    void Run();

private:
    //组合EventLoop
    muduo::net::EventLoop m_eventLoop;
    //注册成功的服务对象和其服务方法的所有信息
    //已经注册的服务，服务名:服务信息(对象，方法)  通过service -> method
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
    //新连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    //读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buf,muduo::Timestamp timestamp);
    //Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,google::protobuf::Message *response);
};