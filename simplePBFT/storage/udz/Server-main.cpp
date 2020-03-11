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
#include "tcp-Server.h"
using namespace std;
std::atomic_llong TotalRecvSize = ATOMIC_VAR_INIT(0);
std::atomic_llong total_client_num = ATOMIC_VAR_INIT(0);
std::atomic_llong total_packet_num = ATOMIC_VAR_INIT(0);
#define TXNUM 8192
#define BUF_N 800
//#define QRY_TIMES 5000
//int judgeblock = 0;
Block go_block;
vector<Node> exe_NO;
void do_one_query(int Sd)
{
    myPkg query_pkg;
    bool found=query_pkgs.try_dequeue(query_pkg);
    if(found)
    {
        //query by code
	timeval dstart,dend;
	gettimeofday(&dstart,NULL);
        int64_t k1=query_pkg.lenc(0);
        int64_t k2=query_pkg.renc(0);
        //searchRangeByCode(k1,k2,query_pkg);
        //query by cipher
        string c1=query_pkg.lcip(0);
        string c2=query_pkg.rcip(0);
        //searchRangeByCipher(Sd,c1,c2,query_pkg);
        //query udz by code
        int msg_id=0;
        searchRangeByCodeUDZ(Sd,k1,k2,query_pkg,msg_id);
        //query udz by cipher
        //searchRangeByCipherUDZ(Sd,c1,c2,query_pkg,msg_id);
        int h=getHeight(root->right);
        for(int i=0;i<h;i++)
         {
           string s=sha256("cipher");
         }
	 gettimeofday(&dend,NULL);
         gettimeofday(&qend,NULL);
	float dtime_use=(dend.tv_sec-dstart.tv_sec)*1000000+(dend.tv_usec-dstart.tv_usec);//微秒
	delay=delay+dtime_use;
         int size=query_pkg.allcip_size();
       	 re_size=size;
        /*query_pkg.set_msgtype(QRY_END);
	std::string str;
        query_pkg.SerializeToString(&str);
        clients[query_pkg.encid()%clients.size()]->send(str);*/
    }
}
void* do_query(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    sleep(25);
    gettimeofday(&qstart,NULL);
    while(1)
    {
        do_one_query(my_data->Sd);
    }


}
int Encode_NO()
{
    enc_req enc;
    bool found=enc_queue.try_dequeue(enc);
    if(found)
    {
	Tx tx;
        tx.tx_type=TX;
        tx.v_bef=0;
        tx.y=0;
        tx.cipher="cipher";
        tx.path="100";
        tx.mtd=0;
	tx_buffer.enqueue(tx);
    }
}
void execute_NO(Tx tx)
{
    Node nd(tx.cipher,tx.y);
    exe_NO.push_back(nd);
    usleep(10);
}

void* do_encode(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    sleep(5);
    gettimeofday(&mystart,NULL);
    bool isGetTime=0;
    while(1)
    {
	//add lqq!
        if(enc_n>=BUF_N && flag==0)
        {
            while(1)
            {
                if(flag==1)
                    break;
            }
        }
        if(flag==1/* && enc_n>=BUF_N*/)
        {
            if(!isGetTime)
            {
                gettimeofday(&buf_start,NULL);
                int num=0;
                Print(num);
		cout<<"num:"<<num<<endl;
                true_n=num+update_time1;
                isGetTime=1;
            }
            Encode_m1(my_data->Sd);
        }
        else
        {
            Encode_m1(my_data->Sd);
            enc_n++;
        }
        //end
        //Encode_m0(my_data->Sd);
    }
}

void* print_tps(void *threadarg)
{
    while(1){
        sleep(10);
        //cout<<"qry size:"<<re_size<<endl;
        /*int num=0;
        Print(num);
        cout<<"total num: "<<num<<endl;
	//cout<<"exe time:"<<delay/1000.0/num<<endl;
        num+=update_time;
        num+=tx_buffer.size_approx();
	//num=exe_NO.size();
        int abort_num=TXNUM-num;
        int conflict_num=abort_num-reb_num;
        cout<<"abort num: "<<abort_num<<", conflict_num: "<<conflict_num<<", reb_num: "<<reb_num<<", update time: "<<update_time<<endl;
        float time_use=(myend.tv_sec-mystart.tv_sec)*1000000+(myend.tv_usec-mystart.tv_usec);//微秒
        printf("time_use is %f microseconds, tx num is %d, tps is about %f\n",time_use,num,(num+1)*1000000.0/time_use);*/
// float qtime_use=(qend.tv_sec-qstart.tv_sec)*1000000+(qend.tv_usec-qstart.tv_usec);//微秒
	//add lqq!
        int num=0;
        Print(num);
	cout<<"true N is about:"<<true_n<<endl;
	num+=update_time1;
	num+=update_time2;
        int true_b=num-true_n;
	int abort_num=TX_PER_BLOCK-true_b;
	cout<<"abort num:"<<abort_num<<endl;
        float time_use=(buf_end.tv_sec-buf_start.tv_sec)*1000000+(buf_end.tv_usec-buf_start.tv_usec);//微秒
        printf("time_use is %f microseconds, true_b is %d, tps is about %f\n",time_use,true_b,(true_b+1)*1000000.0/time_use);
        //end               
// printf("query time_use is %f microseconds, query num is %d, tps is about %f\n",qtime_use,QRY_TIMES,(QRY_TIMES+1)*1000000.0/qtime_use);
		//cout<<"delay is about:"<<delay/1000.0/QRY_TIMES<<endl;
   }
}

void* do_rec(const string& input)
{
//    struct thread_data *my_data;
//    my_data = (struct thread_data *) threadarg;
//    while(1)
//    {
//       myPkg pkg;
//       int n=rec_msg(my_data->Sd,pkg);
//       if(n>0)
//       {
           myPkg pkg;
           pkg.ParseFromString(input);
           if(pkg.msgtype()==CMP_RST)
           {
               int enc_id=pkg.encid();
               vector<myPkg> tmpv;
               rst_list.find(enc_id,tmpv);
               tmpv.push_back(pkg);
               rst_list.insert(enc_id,tmpv);
           }//for query
           else if(pkg.msgtype()==QRY_END)
           {
                /*int size=pkg.allcip_size();
                               int h=getHeight(root);
                               for(int i=0;i<h+size;i++)
                               {
                                   string s=sha256("cipher");
                               }
                               gettimeofday(&qend,NULL);
                               re_sizes.push_back(pkg.allcip_size());*/
           }
           else if(pkg.msgtype()==QRY_CMP_RST || pkg.msgtype()==QRY_SORT_RST)
           {
                int enc_id=pkg.encid();
               vector<myPkg> tmpv;
               qry_rst.find(enc_id,tmpv);
               tmpv.push_back(pkg);
               qry_rst.insert(enc_id,tmpv);
           }//end
           else if(pkg.msgtype()==TX)
           {
               Tx tx;
               tx.tx_type=pkg.msgtype();
               tx.v_bef=pkg.vbef();
               tx.y=pkg.y();
               tx.cipher=pkg.cipher();
               tx.path=pkg.path();
               tx.mtd=pkg.mtd();
               tx_buffer.enqueue(tx);
           }
           else if(pkg.msgtype()==REB_TX)
           {
               if(pkg.mtd()==1)
               {
                   Tx tx;
                   tx.mtd=pkg.mtd();
                   tx.tx_type=pkg.msgtype();
                   tx_buffer.enqueue(tx);
               }
               else
               {
                   Tx tx;
                   tx.tx_type=pkg.msgtype();
                   tx.v_udz=pkg.vudz();
                   tx.cipher=pkg.cipher();
                   tx.mtd=pkg.mtd();
                   for(int i=0;i<pkg.allcip_size();i++)
                   {
                       if(pkg.allcip_size()>0)
                       {
                           tx.lz.list.push_back(Node(pkg.allcip(i),pkg.allenc(i)));
                       }
                   }
                   tx_buffer.enqueue(tx);
               }

           }
           else if(pkg.msgtype()==RESORT_TX)
           {
	       Tx tx;
               //add lqq
               tx.tx_type=pkg.msgtype();
               tx.v_bef=pkg.vbef();
               tx.path=pkg.path();
               tx.cipher=pkg.cipher();
               tx.mtd=pkg.mtd();
               int lsize=pkg.lcip_size();
               int rsize=pkg.rcip_size();
               for(int i=0;i<lsize;i++)
               {
                   if(pkg.lcip(i).length()>0)
                   {
                       tx.lz.list.push_back(Node(pkg.lcip(i),pkg.lenc(i)));
                   }
               }
               for(int i=0;i<rsize;i++)
               {
                   if(pkg.rcip(i).length()>0)
                   {
                       tx.rz.list.push_back(Node(pkg.rcip(i),pkg.renc(i)));
                   }
               }
               tx_buffer.enqueue(tx);
           }
           else
           {
               cerr<<"wrong message type when rec: "<<pkg.msgtype()<<endl;
           }
//       }
//    }
}
void onConnection(const TcpConnectionPtr &conn)
{
    total_client_num++;
    addClientID(total_client_num-1,conn);
}
void onMessage(const TcpConnectionPtr &conn,
               Buffer *buf,
               Timestamp time)
{
    do_rec(buf->retrieveAllAsString());
}
void Init()
{
    /*vector<int> cip_v;
    for(int i=0;i<TXN;i++)
    {
        cip_v.push_back(i);
    }
    random_shuffle(cip_v.begin(),cip_v.end());
    for(int i=0;i<TXN;i++)
    {
        string data="cipher"+to_string(cip_v[i]);
        enc_req enc;
        enc.id=i;
        enc.cipher=data;
        enc_queue.enqueue(enc);
    }*/
    srand(time(NULL));
    for(int i=0;i</*TXNUM*/BUF_N+TX_PER_BLOCK;i++)
    {
        int64_t plain_rnd=dice64(0,TXN-1);
        string data="cipher"+to_string(plain_rnd);
        enc_req enc;
        enc.id=i;
        enc.cipher=data;
        enc_queue.enqueue(enc);
    }
    root=new Node("cipher-1",-1);
    Node* node=new Node("cipher"+to_string(TXN),M);
    root->right=node;
    node->parent=root;
    /*for(int i=0;i<QRY_TIMES;i++)
    {
        myPkg query_pkg;
        query_pkg.set_encid(i);
        query_pkg.set_msgtype(REQ_QRY);
        query_pkg.add_lcip("cipher0");
	int cip_high=8*4300;
        query_pkg.add_rcip("cipher"+to_string(cip_high));
        query_pkg.add_lenc(0);
        query_pkg.add_renc(34359738368*4000);
        query_pkgs.enqueue(query_pkg);
    }*/
    int threadnum = 6;
    EventLoop loop;
    InetAddress addr("127.0.0.1",1234);
    TcpServer server(&loop, addr, "echo");
    server.setConnectionCallback(&onConnection);
    server.setMessageCallback(&onMessage);
    server.setThreadNum(threadnum);
    server.start();
    cout<<"start"<<endl;
    pthread_t tid1[NUM_ENCODE],tid2[NUM_REC],tid3[EXE_TX],tid4[NUM_QRY];
    struct thread_data td1[NUM_ENCODE],td2[NUM_REC],td3[EXE_TX],td4[NUM_QRY];
    for(int i=0; i < NUM_ENCODE; i++ ){
        td1[i].thread_id = i;
//        td1[i].Sd = newSd;
        int rc = pthread_create(&tid1[i], nullptr,do_encode, (void *)&td1[i]);
        pthread_detach(tid1[i]);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    /*for(int i=0; i < NUM_QRY; i++ ){
            td4[i].thread_id = i;
            //td4[i].Sd = newSd;
            int rc = pthread_create(&tid4[i], nullptr,do_query, (void *)&td4[i]);
            pthread_detach(tid4[i]);
            if (rc){
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }*/
        pthread_t tid5;
        struct thread_data td5;
//        td4.Sd=newSd;
        int rc = pthread_create(&tid5, nullptr,print_tps, (void *)&td5);
        pthread_detach(tid5);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
//    pthread_t tid4;
//    struct thread_data td4;
//    td4.Sd=newSd;
//    int rc = pthread_create(&tid4, nullptr,do_query, (void *)&td4);
//    pthread_detach(tid4);
//    if (rc){
//        cout << "Error:unable to create thread," << rc << endl;
//        exit(-1);
//    }
    loop.loop();
    while(1){
    usleep(1);
    }
    //return 0;
}

int getProcessedBlock(){
    Block new_block;
    int count = 0;
    //timeval estart,eend;
    while(1)
    {
        Tx tx;
        bool found=tx_buffer.try_dequeue(tx);
        if(found)
        {
	    count++;
	    //gettimeofday(&estart,NULL);
	    execute_by_type(tx,new_block);
	    //execute_NO(tx);
	    //gettimeofday(&eend,NULL);
	    //float etime_use=(eend.tv_sec-estart.tv_sec)*1000000+(eend.tv_usec-estart.tv_usec);//微秒
	    gettimeofday(&myend,NULL);
	    //delay=delay+etime_use;
		//add lqq!
                exe_n++;
                if(exe_n>=BUF_N)
                    flag=1;
                gettimeofday(&buf_end,NULL);
                //end
	    if(count>=TX_PER_BLOCK && new_block.n>0){
        	   go_block = new_block;
		   usleep(100000);
		   return 1;
	    }	
	}
    }
    /*Block new_block;
    new_block.block_id=block_id;
    for(int i=0;i<tmp_block.n;i++){
        Tx tx1 = tmp_block.get(i);
        execute_by_type(tx1,new_block);
        gettimeofday(&myend,NULL);
    }
    if(new_block.n>0){
        block_id++;
        go_block = new_block;
	    return 1;
       //return serialize_block(new_block);
    }else{
        //string ss="";
        return 0;
        //return ss.c_str();
    }*/
}

const char * Judge(){
    return serialize_block(go_block);
}
