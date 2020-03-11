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
using namespace std;
void do_one_query(int Sd)
{
    myPkg query_pkg;
    bool found=query_pkgs.try_dequeue(query_pkg);
    if(found)
    {
        //query by code
        int64_t k1=query_pkg.lenc(0);
        int64_t k2=query_pkg.renc(0);
//        searchRangeByCode(k1,k2,query_pkg);
        //query by cipher
//        string c1=query_pkg.lcip(0);
//        string c2=query_pkg.rcip(0);
//        searchRangeByCipher(Sd,c1,c2,query_pkg);
        //query udz by code
        int msg_id=0;
        searchRangeByCodeUDZ(Sd,k1,k2,query_pkg,msg_id);
        //query udz by cipher
//        int msg_id=0;
//        searchRangeByCipherUDZ(Sd,c1,c2,query_pkg,msg_id);
        query_pkg.set_msgtype(QRY_END);
        send_msg(Sd,query_pkg);
    }
}
void* do_query(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    sleep(12);
    gettimeofday(&qstart,NULL);
    while(1)
    {
        do_one_query(my_data->Sd);
    }


}
void* write_tree(void *threadarg)
{
//    char *serverIp1 = "127.0.0.1"; int port1 =8081/* atoi(argv[2])*/;
//    struct hostent* host1 = gethostbyname(serverIp1);
//    sockaddr_in sendSockAddr1;
//    bzero((char*)&sendSockAddr1, sizeof(sendSockAddr1));
//    sendSockAddr1.sin_family = AF_INET;
//    sendSockAddr1.sin_addr.s_addr =inet_addr(inet_ntoa(*(struct in_addr*)*host1->h_addr_list));
//    sendSockAddr1.sin_port = htons(port1);
//    int clientSd1 = socket(AF_INET, SOCK_STREAM, 0);
//    //try to connect...
//    int status = connect(clientSd1,(sockaddr*) &sendSockAddr1, sizeof(sendSockAddr1));
//    if(status < 0)
//    {
//        cout<<"Error connecting to socket!"<<endl;
//    }
//    cout << "Connected to the Node!" << endl;
    while(1)
    {

//        Block tmp_block;
//        int count = 0;
//        while(1)
//        {
//            Tx tx;
//            bool found=tx_buffer.try_dequeue(tx);
//            if(found)
//            {
//                tmp_block.push(tx);
//                count++;
//                if(count>=TX_PER_BLOCK){
//                    break;
//                }
//            }
//        }
//        Block new_block;
//        new_block.block_id=block_id;
//        for(int i=0;i<tmp_block.n;i++){
//            Tx tx1 = tmp_block.get(i);
//            execute_by_type(tx1,new_block);
//            gettimeofday(&myend,NULL);
//        }
//        if(new_block.n>0){
////            send_block(clientSd1,new_block);
//            usleep(100000);
//        }
//        block_id++;
        int count = 0;
        Block new_block;
        while(1)
        {
            Tx tx;
            bool found=tx_buffer.try_dequeue(tx);
            if(found)
            {
                count++;
                execute_by_type(tx,new_block);
                gettimeofday(&myend,NULL);
                if(count>=TX_PER_BLOCK && new_block.n>0){
                    new_block.block_id=block_id;
                    block_id++;
                    usleep(100000);
                    break;
                }
            }

        }

    }

}
void* do_encode(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    sleep(1);
    gettimeofday(&mystart,NULL);
    while(1)
    {
        while(is_reb)
        {

        }
        is_enc=1;
        Encode_m0(my_data->Sd);
        is_enc=0;
//        nn++;
    }
}

void print_tps()
{
//    while(1){

        sleep(15);
        for(int i=0;i<re_sizes.size();i++)
            cout<<re_sizes[i]<<endl;
        int num=0;
        Print(num);
        num+=update_time;
        num+=tx_buffer.size_approx();
        abort_num=TXNUM-num;
        conflict_num=abort_num-reb_num;
        cout<<"total num: "<<num<<endl;
        cout<<"abort num: "<<abort_num<<", conflict_num: "<<conflict_num<<", reb_num: "<<reb_num<<", update time: "<<update_time<<" txnum: "<<nn<<endl;
        float time_use=(myend.tv_sec-mystart.tv_sec)*1000000+(myend.tv_usec-mystart.tv_usec);//微秒
        printf("time_use is %f microseconds, tx num is %d, tps is about %f\n",time_use,num,(num+1)*1000000.0/time_use);
//    }
}

void* do_rec(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    while(1)
    {
       myPkg pkg;
       int n=rec_msg(my_data->Sd,pkg);
       if(n>0)
       {
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
               int size=pkg.allcip_size();
               int h=getHeight(root);
               for(int i=0;i<h+size;i++)
               {
                   string s=sha256("cipher");
               }
               gettimeofday(&qend,NULL);
               re_sizes.push_back(pkg.allcip_size());
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
                   tx.mtd=1;
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
                           //cout<<"tx left udz:"<<tx.lz.list[i].cipher<<", "<<tx.lz.list[i].code<<endl;
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
                       //cout<<"tx left udz:"<<tx.lz.list[i].cipher<<", "<<tx.lz.list[i].code<<endl;
                   }
               }
               for(int i=0;i<rsize;i++)
               {
                   if(pkg.rcip(i).length()>0)
                   {
                       tx.rz.list.push_back(Node(pkg.rcip(i),pkg.renc(i)));
                       //cout<<"tx right udz:"<<tx.rz.list[i].cipher<<", "<<tx.rz.list[i].code<<endl;
                   }
               }
               tx_buffer.enqueue(tx);
           }
           else
           {
               cerr<<"wrong message type when rec: "<<pkg.msgtype()<<", n="<<n<<endl;
           }
       }
    }
}

int main(int argc, char *argv[])
{
//    vector<int> cip_v;
//    for(int i=0;i<PLSIZE;i++)
//    {
//        cip_v.push_back(i);
//    }
//    random_shuffle(cip_v.begin(),cip_v.end());
//    for(int i=0;i<PLSIZE;i++)
//    {
//        string data="cipher"+to_string(cip_v[i]);
//        enc_req enc;
//        enc.id=i;
//        enc.cipher=data;
//        enc_queue.enqueue(enc);
//    }
    srand(time(NULL));
    for(int i=0;i<TXNUM;i++)
    {
        int64_t plain_rnd=dice64(0,PLSIZE-1);
        string data="cipher"+to_string(plain_rnd);
        enc_req enc;
        enc.id=i;
        enc.cipher=data;
        enc_queue.enqueue(enc);
    }
    root=new Node("cipher-1",-1);
    Node* node=new Node("cipher"+to_string(PLSIZE),M);
    root->right=node;
    node->parent=root;
//    for(int i=0;i<100;i++)
//    {
//        myPkg query_pkg;
//        query_pkg.set_encid(i);
//        query_pkg.set_msgtype(REQ_QRY);
////        query_pkg.add_lcip("cipher500");
////        query_pkg.add_rcip("cipher800");
//        query_pkg.add_lenc(100);
//        query_pkg.add_renc(81474976710656);
//        query_pkgs.enqueue(query_pkg);
//    }
    //grab the port number
    int port = 8080/*atoi(argv[1])*/;
    //buffer to send and receive messages with

    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = ::bind(serverSd, (struct sockaddr*) &servAddr,
                          sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    cout << "Waiting for a client to connect..." << endl;
    //listen for up to 5 requests at a time
    listen(serverSd, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    //accept, create a new socket descriptor to
    //handle the new connection with client
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Connected with client!" << endl;
    pthread_t tid1[NUM_ENCODE],tid2[NUM_REC],tid3[EXE_TX],tid4[NUM_QRY];
    struct thread_data td1[NUM_ENCODE],td2[NUM_REC],td3[EXE_TX],td4[NUM_QRY];
    for(int i=0; i < NUM_ENCODE; i++ ){
        td1[i].thread_id = i;
        td1[i].Sd = newSd;
        int rc = pthread_create(&tid1[i], nullptr,do_encode, (void *)&td1[i]);
        pthread_detach(tid1[i]);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    for(int i=0; i < NUM_REC; i++ ){
            td2[i].thread_id = i;
            td2[i].Sd = newSd;
            int rc = pthread_create(&tid2[i], nullptr,do_rec, (void *)&td2[i]);
            pthread_detach(tid2[i]);
            if (rc){
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }
    for(int i=0; i < EXE_TX; i++ ){
        td3[i].thread_id = i;
        int rc = pthread_create(&tid3[i], nullptr,write_tree, (void *)&td3[i]);
        pthread_detach(tid3[i]);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
//    for(int i=0; i < NUM_QRY; i++ ){
//        td4[i].thread_id = i;
//        td4[i].Sd = newSd;
//        int rc = pthread_create(&tid4[i], nullptr,do_query, (void *)&td4[i]);
//        pthread_detach(tid4[i]);
//        if (rc){
//            cout << "Error:unable to create thread," << rc << endl;
//            exit(-1);
//        }
//    }
    while(1){
        print_tps();
    }
    return 0;
}
