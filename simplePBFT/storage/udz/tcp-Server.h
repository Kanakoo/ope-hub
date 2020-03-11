#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "concurrentqueue.h"
#include "concurrentmap.h"
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
#include <math.h>
#include <mutex>
#include <time.h>
#include <queue>
#include <atomic>
#include <shared_mutex>
#include <assert.h>
#include <algorithm>
#include "util.h"
#include "sha256.h"
using namespace std;

#include <muduo/net/TcpServer.h>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

using namespace muduo;
using namespace muduo::net;

//add lqq
//variables
#define NUM_ENCODE 16
#define EXE_TX 1
#define NUM_REC 4
#define NUM_QRY 0
//add lqq!
extern timeval buf_start;
extern timeval buf_end;
extern std::atomic<int> true_n;
extern std::atomic<bool> flag;
extern std::atomic<int> enc_n;
extern std::atomic<int> exe_n;
extern std::atomic<int> update_time1;
extern std::atomic<int> update_time2;
//end
extern std::atomic<bool> is_reb;
extern std::atomic<bool> is_enc;
extern std::atomic<int> version;
extern std::atomic<int> version_udz;
extern std::atomic<int> reb_num;
extern std::atomic<bool> method;
extern Node* root;
extern std::atomic<int> update_time;
extern moodycamel::ConcurrentQueue<myPkg> query_pkgs;
//HZC
extern CTSL::HashMap<int,vector<myPkg>> rst_list;
extern moodycamel::ConcurrentQueue<enc_req> enc_queue;
extern moodycamel::ConcurrentQueue<Tx> tx_buffer;
extern Block tx_block;
extern std::atomic<int> block_id;
extern CTSL::HashMap<int,vector<myPkg>> qry_rst;
extern std::atomic<int> re_size;
extern timeval qstart;
extern timeval qend;
extern std::atomic<float> delay;
//HZC
extern timeval mystart;
extern timeval myend;
void quicksort(vector<string>& tmp_list,map<string,int64_t>& tmp_map,int low, int high);
void Inorder(Node* node,vector<string>& tmp_list,map<string,int64_t>& tmp_map);
void Inorder(vector<string>& tmp_list,map<string,int64_t>& tmp_map, vector<Node>& all_udz);
void Inorder1(Node* node,int& num);
void Print(int& num);
void Inorder2(Node* node,myPkg& item);
void get_all_udz(myPkg& item);
int getHeight(Node* root);
Node* CreateBST(int low,int high,vector<Node> A);
void destory(Node*& p);
void destory();

void traverseByCode(Node* root,int64_t k1,int64_t k2,myPkg& pkg);
void searchRangeByCode(int64_t k1,int64_t k2,myPkg& pkg);
void searchRangeByCipher(int Sd,string c1, string c2,myPkg& qry_pkg);

Node* lowestCommonAncestor(Node* root, Node* p, Node* q);
void searchOneByCipherUDZ(int Sd,string c,int64_t& k,Node*& p,int& msg_id,int qry_id);
void searchOneByCodeUDZ(int Sd,int64_t k,Node*& p,int& msg_id,int qry_id);
void traverseByCodeUDZ(int Sd,Node* root,int& msg_id,int qry_id);
void searchRangeByCipherUDZ(int Sd,string c1,string c2,myPkg& pkg,int& msg_id);
void searchRangeByCodeUDZ(int Sd,int64_t k1,int64_t k2,myPkg& pkg,int& msg_id);
void Update(vector<string>& data,map<string,int64_t>& tmp_map);
void Update(vector<string>& data,map<string,int64_t>& tmp_map);
//int Encode(int newSd);
void execute_tx_m0(Tx& tx,Block& tx_block);
int64_t Encode1(string current,map<string,int64_t>& tmp_map);
int Encode_m0(int Sd);
//change next
int Encode_m1(int Sd);
void execute_tx_m1(Tx& tx,Block& tx_block);
void execute_by_type(Tx& tx,Block& tx_block);
void Inorder4(Node* node,vector<string>& tmp_list,map<string,int64_t>& tmp_map);
void Inorder4(vector<string>& tmp_list,map<string,int64_t>& tmp_map);
int64_t dice64(int64_t begin, int64_t end);
int64_t compute_rand_code(int64_t y1,int64_t y2);
int compute_one(bool is_l,string cipher,Node* parent,myPkg& pkg);
//muduo
extern std::map<int,TcpConnectionPtr> clients;
void addClientID(int index, const TcpConnectionPtr &con);
void Init();
int getProcessedBlock();
const char* Judge();
#endif // TCPSERVER_H