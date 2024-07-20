#include<iostream>
#include<string>
#include"test.pb.h"
using namespace fixbuf;
int main(){
    GetFriendList rsp;
    ResultCode *rc = rsp.mutable_err();
    rc->set_errcode(0);
    rc->set_errmsg("哈哈");

    User *user1 = rsp.add_friend_list();
    user1->set_age(18);
    user1->set_name("zhangsan");
    user1->set_sex(User::Man);

    std::cout<<rsp.friend_list_size()<<std::endl;
    std::cout<<rsp.friend_list(0).age()<<std::endl;
    std::cout<<rsp.friend_list(0).name()<<std::endl;
    std::cout<<rsp.friend_list(0).sex()<<std::endl;
    return 0;
}




int main1(){
    LoginRq rq;
    rq.set_name("Michcle_Jackson");
    rq.set_pwd("kingofpop");

    std::string send_str;
    if(rq.SerializeToString(&send_str)){
        //如果序列化成功，send_str里面就有值了
        std::cout<<"发送的数据:"<<send_str<<std::endl;
    }

    LoginRq rs;
    //反序列化
    if(rs.ParseFromString(send_str)){
        std::cout<<rs.name()<<std::endl;
        std::cout<<rs.pwd()<<std::endl;
    }
    
    return 0;
}