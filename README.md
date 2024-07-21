## 编译
运行环境为Ubuntu22.04
依赖库:protobuf muduo

进入build 目录下 cmake ..

make生成可执行文件provider  consumer

./provider -i test.conf

./consumer-i test.conf

运行即可
