#include"mprpcconfig.h"
#include<iostream>
#include<string>

//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file){
    FILE *pf = fopen(config_file,"r");
    if(nullptr == pf){
        std::cout<<config_file<<"is not exist!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    //三种情况，注释，正确配置项，开头和末尾多余空格，空行
    while(!feof(pf)){
        char buf[512] = {0};
        fgets(buf,512,pf);

        //去除开头和末尾的空行
        std::string read_buf(buf);
        Trim(read_buf);

        if(read_buf[0] == '#'||read_buf.empty()){
            continue;
        }

        //解析配置项
        int idx = read_buf.find('=');
        if(idx == -1) continue; //此条配置项不合法

        std::string key;
        std::string value;

        key = read_buf.substr(0,idx);
        //由于配置项不一定有没有空行例如
        //   rpcserverip      =   192.168.1.2
        //此种情况也算对，所以需要分别去除空行
        Trim(key);

        //从idx也就是=的位置开始找，找到'\n'的位置
        int endidx = read_buf.find('\n',idx);
        //-1是为了不要换行符
        value = read_buf.substr(idx+1,endidx-idx-1);
        //去掉换行再取两头空格
        Trim(value);
        m_configMap.insert({key,value});
    }

    fclose(pf);
}

//查询配置项信息
std::string MprpcConfig::Load(const std::string &key){
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        //没查到对应的配置
        return {};
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf){
    //第一个不是空格的元素
    int idx = src_buf.find_first_not_of(' ');
    if(idx != -1){
        //说明字符串前面有空格
        src_buf = src_buf.substr(idx,src_buf.size()-idx);
    }
    idx = src_buf.find_last_not_of(' ');
    if(idx != -1){
        //说明字符串后面有空格
        src_buf = src_buf.substr(0,idx+1);
    }
}