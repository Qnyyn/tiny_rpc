#include<iostream>
#include"mprpcapplication.h"
#include"user.pb.h"
#include"mprpcchannel.h"
#include"friend.pb.h"

int main(int argc,char **argv){
    //服务器ip端口，zk ip端口
    MprpcApplication::Init(argc,argv);

    //演示远程调用发布的rpc方法Login
    //通过桩类来进行数据的序列化和网络的发送
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //stub.Login();//RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    //rpc方法的响应
    fixbug::LoginResponse response;

    stub.Login(nullptr,&request,&response,nullptr);

    //一次rpc调用完成，读调用的结果
    if(0 == response.result().errcode()){
        std::cout<<"rpc login response success:"<<response.success()<<std::endl;
    }
    else{
        std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
    }

    fixbug::RegisterRequest register_request;
    register_request.set_id(3000);
    register_request.set_name("zhang san");
    register_request.set_pwd("123456");

    //rpc方法的响应
    fixbug::RegisterResponse register_response;

    stub.Register(nullptr,&register_request,&register_response,nullptr);

    //一次rpc调用完成，读调用的结果
    if(0 == response.result().errcode()){
        std::cout<<"rpc Register response success:"<<register_response.success()<<std::endl;
    }
    else{
        std::cout<<"rpc Register response error:"<<register_response.result().errmsg()<<std::endl;
    }

    friendbug::FriendServiceRpc_Stub stub_friend(new MprpcChannel());

    //封装请求好友列表请求
    friendbug::GetFriendListRequest friend_request;
    friend_request.set_userid(20);
    
    friendbug::GetFriendListResponse friend_response;

    //后加入的controller，传递错误信息
    MprpcController controller;

    //发起rpc方法调用
    stub_friend.GetFriendList(&controller,&friend_request,&friend_response,nullptr);
    //通过控制器检查错误情况s
    if(controller.Failed()){
        std::cout<<controller.ErrorText()<<std::endl;
        return 0;
    }

    //调用返回
    if(0 == friend_response.result().errcode()){
        std::cout<<"rpc GetFriendList response success!"<<std::endl;
        int size = friend_response.friends_size();
        for(int i = 0;i < size;i++){
            std::cout<<"index:"<<i+1<<" name:"<<friend_response.friends(i)<<std::endl;
        }
    }
    else{
        std::cout<<"rpc GetFriendList response error: "<<response.result().errmsg()<<std::endl;
    }

    return 0;
}