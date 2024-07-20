#include"mprpccontroller.h"

MprpcController::MprpcController(){
    //构造时默认不出错
    m_failed = false;
    m_errText = "";
}
void MprpcController::Reset(){
    m_failed = false;
    m_errText = "";
}
bool MprpcController::Failed() const{
    //判断是否成功
    return m_failed;
}
std::string MprpcController::ErrorText() const{
    return m_errText;
}
void MprpcController::SetFailed(const std::string& reason){
    m_failed = true;
    m_errText = reason;
}

//目前未实现的具体的功能,必须给出来，不然纯虚函数无法实例化对象
void  MprpcController::StartCancel(){

}
bool MprpcController::IsCanceled() const{
    return false;
}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){

}