#include<iostream>
#include<string>
#include"user.pb.h"
#include"mprpcapplication.h"
#include"rpcprovider.h"
#include"friend.pb.h"
#include"logger.h"
/*
UserService是本地服务，提供Login方法
*/
class UserService:public fixbug::UserServiceRpc{//RPC服务提供者
public:
    //本地login
    bool Login(std::string name,std::string pwd){
        std::cout<<"doing local service:Login "<<std::endl;
        std::cout<<"name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }
    //本地register
    bool Register(uint32_t id,std::string name,std::string pwd){
        std::cout<<"doing local service:Register "<<std::endl;
        std::cout<<"id:"<<id<<" name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    //重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
    void Login(::google::protobuf::RpcController* controller,
                         const ::fixbug::LoginRequest*request,
                         ::fixbug::LoginResponse* response,
                         ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数LoginRequest 应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //调用本地业务
        bool login_result = Login(name,pwd);

        //写响应，传框架执行
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作，执行响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                         const ::fixbug::RegisterRequest*request,
                         ::fixbug::RegisterResponse* response,
                         ::google::protobuf::Closure* done)
    {  
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool success = Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(success);

        done->Run();
    }
};

class FriendService:public friendbug::FriendServiceRpc{
public:
    //本地获取好友列表
    std::vector<std::string> GetFriendList(uint32_t userid){
        std::cout<<"do local GetFriendList service!"<<" userid:"<<userid<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }

    //重写protobuf方法
    void GetFriendList(::google::protobuf::RpcController* controller,
                         const ::friendbug::GetFriendListRequest* request,
                         ::friendbug::GetFriendListResponse* response,
                         ::google::protobuf::Closure* done)
    {
        //接收请求
        uint32_t userid = request->userid();
        //做本地调用
        std::vector<std::string> friendList = GetFriendList(userid);
        //打包回复
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name:friendList){
            std::string *p = response->add_friends();
            *p = name;
        }
        //发送回复
        done->Run();
    }

};


int main(int argc,char **argv){
    LOG_INFO("first log message!!");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    //callee方如何提供和发布服务呢，通过框架，那么框架要运行起来吧

    //先调用初始化框架,从配置文件读取服务器以及zookeeper ip和端口
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象,把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点 run运行，进程进入阻塞状态，等待远端(caller)发起rpc调用请求
    provider.Run();
    return 0;
}