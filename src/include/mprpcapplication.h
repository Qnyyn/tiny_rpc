#pragma once
#include"mprpcconfig.h"
#include"mprpccontroller.h"
//mprpc框架的基础类，负责初始化操作
class MprpcApplication{
public:
    static void Init(int argc,char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(const MprpcApplication&&) = delete;
    //MprpcApplication& operator=(){}
};