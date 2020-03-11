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
#include "tcp-Client.h"
using namespace std;
//for encrypt and decrypt
map<string,int64_t> text;//cipher-plain
//cache
map<int,int> cache;//plain-encode
std::mutex mtx_cmp;
std::queue<myPkg> cmp_buffer;
myPkg query_pkg;
//EventBase eventbase;
int64_t cmp_cli(string ciphertxt,string cipherx)
{
    if(text[cipherx]<-1 ||text[cipherx]>PLSIZE)
        return ERR;
    return text[ciphertxt]-text[cipherx];
}
//bool sortFunCip(const string &p1, const string &p2)
//{
//    string s1=p1;
//    string s2=p2;
//    return text[s1] < text[s2];//升序排列
//}
