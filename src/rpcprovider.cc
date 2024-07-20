#include"rpcprovider.h"
#include"mprpcapplication.h"
#include"rpcheader.pb.h"
#include"logger.h"
#include"zookeeperutil.h"

//框架给外部使用，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service){
    //这里参数的service在注册的时候通过子类指针可以被接收
    //从而找到具体的service进行发布

    ServiceInfo service_info;

    //获取服务对象的描述信息(为了获取服务的类名)
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();
    //获取服务名
    std::string service_name = pServiceDesc->name();
    //获取服务对象service的方法数量
    int methodCnt = pServiceDesc->method_count();
    //std::cout<<"service_name :"<<service_name<<std::endl;
    LOG_INFO("service_name:%s",service_name.c_str());

    for(int i = 0;i < methodCnt;i++){
        //获取服务对象的方法描述  为了得到方法名字
        const google::protobuf::MethodDescriptor *pMethodDesc = pServiceDesc->method(i);
        std::string method_name = pMethodDesc->name();
        //有了方法名和方法描述  就能调方法了  可以保存到map里
        service_info.m_methodMap.insert({method_name,pMethodDesc});

        //std::cout<<"method name:"<<method_name<<std::endl;
        LOG_INFO("method_name:%s",method_name.c_str());
    }

    //保存每个服务自己的  服务对象(Object)和方法映射表
    service_info.m_service = service;
    this->m_serviceMap.insert({service_name,service_info});
    
}

//启动rpc服务节点,开始提供rpc远程网络调用服务
void RpcProvider::Run(){
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = std::atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    
    //创建tcpserver对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");

    //绑定回调函数
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));

    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    
    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();
}

//新连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn){
    if(!conn->connected()){
        conn->shutdown();
    }
}

//数据流示例 18UserServiceLogin12namepassword  
//18就是四个字节的协议头长度
//substr截取到UserServiceLogin12拿到协议里的三个内容
//读取出来之后在substr(4+18,12);
//拿到参数

//读写事件回调,如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buf,muduo::Timestamp timestamp){
    //从网络上接收远程rpc调用请求的字符流  muduo提供  Login args
    std::string recv_buf = buf->retrieveAllAsString();

    //从字符流中读取前四个字节的内容,协议头长度 service_name method_name args_size
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);

    //根据header_size 读取数据头的原始字符流  反序列化数据 得到rpc请求的详细信息
    std::string rpc_head_str = recv_buf.substr(4,header_size);

    //反序列化  提取数据
    mprpc::RpcHeader rpcHeader;

    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if(rpcHeader.ParseFromString(rpc_head_str)){
        //反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else{
        //数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_head_str<<"parse error!"<<std::endl;
        return;       
    }

    //获取调用的方法参数
    std::string args_str = recv_buf.substr(4 + header_size,args_size);

    //打印调试信息
    std::cout<<"====================================================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_head_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"====================================================="<<std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        std::cout<<service_name<<"is not exist!"<<std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end()){
        std::cout<<service_name<<":"<<method_name<<"is not exist!"<<std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;//获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second;//获取method对象  Login
    //生成rpc方法调用的请求request和响应response参数(反序列化)
    //! 这里message类型是抽象的，不能具体给出请求类型，因为是框架，具体请求类型都是继承自Message来的
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        //把远端发来的参数反序列化为对应的请求类型
        std::cout<<"request parse error,content:"<<args_str<<std::endl;
        return;
    }

    //参数为什么传一个method，什么方法有具体对应的请求和响应类型，不同方法的请求响应类型可能不同，所以需要传入具体方法
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //在框架上根据远端的rpc请求，调用当前节点上发布的方法
    //new UserService().Login(controller,request,response,done);
    //这里的Login不是本地方法，而是重写的方法，以便框架能通过protobuf调用
    
    //! 给下面的method方法的调用，绑定一个Closure的回调函数(远端请求调用完的返回)
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                const muduo::net::TcpConnectionPtr&, 
                                                                google::protobuf::Message*>
                                                                (this, 
                                                                &RpcProvider::SendRpcResponse, 
                                                                conn, response);
    //最后一个参数是Closure done，就是我们调用完一个本地方法后得返回响应，通过什么返回，返回哪里去
    //就是这个done帮我们做的，我们需要重写内部的run方法(也可以直接用protobuf里面提供的回调)
    service->CallMethod(method,nullptr,request,response,done);

}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,google::protobuf::Message *response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){
        //序列化成功，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else{
        std::cout<<"serialize response_str error!"<<std::endl;
    }
    conn->shutdown();
}
