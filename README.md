## 编译
运行环境为Ubuntu22.04
依赖库:protobuf muduo zookeeper

进入build 目录下 cmake ..

make生成可执行文件provider  consumer

在bin目录，更改配置文件的信息，提供ZK的ip和端口

./provider -i test.conf

./consumer-i test.conf

运行即可
