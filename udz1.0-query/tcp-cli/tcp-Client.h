#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <time.h>
//#include <algorithm>
#include <queue>
#include "../common/util.h"
//#include "../common/myaes.h"
// #include <muduo/net/TcpServer.h>
// #include <muduo/base/AsyncLogging.h>
// #include <muduo/base/Logging.h>
// #include <muduo/base/Thread.h>
// #include <muduo/net/EventLoop.h>
// #include <muduo/net/EventLoopThreadPool.h>
// #include <muduo/net/InetAddress.h>
// #include <muduo/net/TcpClient.h>

//using namespace muduo;
//using namespace muduo::net;
//add lqq
#define NUM_REC 4
//for encrypt and decrypt
extern map<string,int64_t> text;//cipher-plain
//cache
extern map<int,int> cache;//plain-encode
extern std::mutex mtx_cmp;
extern std::queue<myPkg> cmp_buffer;
extern myPkg query_pkg;
//extern EventBase eventbase;
int64_t cmp_cli(string ciphertxt,string cipherx);
//bool sortFunCip(const string &p1, const string &p2);
#endif // TCPCLIENT_H
