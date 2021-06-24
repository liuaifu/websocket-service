# websocket-service
基于boost.beast实现的一个websocket服务示例。

# 依赖
- cmake 2.8.12+
- c++14+
- boost1.66+
- libmysqlclient
- [libdbpool](https://github.com/liuaifu/libdbpool "libdbpool")

# 编译
Linux

	$ git clone https://github.com/liuaifu/websocket-service.git
	$ cd websocket-service
	$ mkdir build
	$ cd build
	$ cmake -DBOOST_ROOT=~/boost_1_66_0/ ..
	$ make
	$ cp ../bin/config.xml .
	$ ./websocket-service
	08:37:20.688291 <info> --------------------------------------------------
	08:37:20.689085 <info> compiled at Oct 11 2018 08:36:22
	08:37:20.689855 <info> log_level: 0
	08:37:20.701466 <info> listening on 0.0.0.0:8888 ...

Windows

	> cmake -G "Visual Studio 15 WIN32" -DBOOST_INCLUDEDIR=x:\boost_1_66_0\include -DBOOST_LIBRARYDIR=x:\boost_1_66_0\lib -DDBPOOL_INCLUDEDIR=x:\libdbpool\include -DMYSQL_INCLUDEDIR=x:\MySQL-5.7\include -DMYSQL_LIBDIR=x:\MySQL-5.7\lib ..
	> cmake -G "Visual Studio 16 2019" -AWIN32 -DBOOST_INCLUDEDIR=x:\boost_1_66_0\include -DBOOST_LIBRARYDIR=x:\boost_1_66_0\lib -DDBPOOL_INCLUDEDIR=x:\libdbpool\include -DMYSQL_INCLUDEDIR=x:\MySQL-5.7\include -DMYSQL_LIBDIR=x:\MySQL-5.7\lib ..
然后在VS中打开工程文件并编译
