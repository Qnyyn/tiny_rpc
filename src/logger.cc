#include"logger.h"
#include<time.h>
#include<iostream>

Logger::Logger(){
    //启动专门的写日志线程
    std::thread writeLogTask([&](){
        for(;;){
            //获取当前的日期，然后取日志信息，写入相应文件中
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);

            //追加方式打开
            FILE *pf = fopen(file_name,"a+");
            if(nullptr == pf){
                std::cout<<"logger file:"<<file_name<<"open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            //弹出日志信息
            std::string msg = m_lckQue.Pop();
            
            char time_buf[128] = {0};
            sprintf(time_buf,"%d:%d:%d => [%s]",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_logLevel == INFO?"info":"error"));
            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(),pf);
            fclose(pf);

        }
    });

    writeLogTask.detach();
}

//获取单例
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}
//设置日志级别
void Logger::SetLogLevel(LogLevel level){
    m_logLevel = level;
}
//写日志到队列里
void Logger::Log(std::string msg){
    m_lckQue.Push(msg);
}