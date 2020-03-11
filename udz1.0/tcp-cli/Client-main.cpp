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
// using namespace muduo;
// using namespace muduo::net;

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
            if(pkg.msgtype()==REQ_CMP)
            {
                //cout<<"rec:"<<rec_item.enc_id<<","<<rec_item.msg_id<<","<<rec_item.cipher<<","<<rec_item.tmp<<endl;
                string tmp=pkg.tmp();
                string cipherx=pkg.cipher();
                string path=pkg.path();
                int64_t cmp=cmp_cli(tmp,cipherx);
                myPkg sen_rst=pkg;;
                sen_rst.set_msgtype(CMP_RST);
                sen_rst.set_rst(cmp);
                if(cmp>0)
                    path+="0";
                else if(cmp<0)
                    path+="1";
                else
                    path="0";
                sen_rst.set_path(path);
                send_msg(my_data->Sd,sen_rst);
            }//for query
            else if(pkg.msgtype()==QRY_END)
            {
                query_pkg=pkg;
            }
            else if(pkg.msgtype()==REQ_CMP_QRY)
            {
                string tmp=pkg.tmp();
                string cipherx=pkg.cipher();
                myPkg sen_rst=pkg;
                int64_t cmp=cmp_cli(tmp,cipherx);
                sen_rst.set_rst(cmp);
                sen_rst.set_msgtype(QRY_CMP_RST);
                send_msg(my_data->Sd,sen_rst);
            }
            else if(pkg.msgtype()==REQ_SORT_QRY)
            {
                myPkg sorted_item=pkg;
                sorted_item.set_msgtype(QRY_SORT_RST);
                vector<string> lcip;
                vector<int64_t> lenc;
                vector<string> rcip;
                vector<int64_t> renc;
                int64_t cmp=sorted_item.rst();
                if(cmp>0)
                {
                    sorted_item.clear_lcip();
                    sorted_item.clear_lenc();
                    int lsize=pkg.lcip_size();
                    for(int i=0;i<lsize;i++)
                    {
                        string s1=pkg.lcip(i);
                        int64_t e1=pkg.lenc(i);
                        lcip.push_back(s1);
                        lenc.push_back(e1);
                    }
                    int l_size=lcip.size();
                    assert(lsize==l_size);

                    //another sort
                    map<int64_t,string> tmp_map;
                    vector<int64_t> plain_list;
                    for(int i=0;i<l_size;i++)
                    {
                        string cipher=lcip.at(i);
                        int64_t plain=text[cipher];
                        tmp_map[plain]=cipher;
                        plain_list.push_back(plain);
                    }
                    sort(plain_list.begin(),plain_list.end());
                    //sort end

                    sort(lenc.begin(),lenc.end());

                    for(int i=0;i<l_size;i++)
                    {
                        sorted_item.add_lcip(tmp_map[plain_list.at(i)]);
                        sorted_item.add_lenc(lenc[i]);
                    }
                    send_msg(my_data->Sd,sorted_item);
                }
                else
                {
                    sorted_item.clear_rcip();
                    sorted_item.clear_renc();
                    int rsize=pkg.rcip_size();
                    for(int i=0;i<rsize;i++)
                    {
                        string s2=pkg.rcip(i);
                        int64_t e2=pkg.renc(i);
                        rcip.push_back(s2);
                        renc.push_back(e2);
                    }
                    int r_size=rcip.size();
                    assert(rsize==r_size);
                    map<int64_t,string> tmp_map;
                    vector<int64_t> plain_list;
                    for(int i=0;i<r_size;i++)
                    {
                        string cipher=rcip.at(i);
                        int64_t plain=text[cipher];
                        tmp_map[plain]=cipher;
                        plain_list.push_back(plain);
                    }
                    sort(plain_list.begin(),plain_list.end());
                    //sort end

                    sort(renc.begin(),renc.end());

                    for(int i=0;i<r_size;i++)
                    {
                        sorted_item.add_rcip(tmp_map[plain_list.at(i)]);
                        sorted_item.add_renc(renc[i]);
                    }
                    send_msg(my_data->Sd,sorted_item);
                }
            }//end
            else if(pkg.msgtype()==REB_TX)
            {
                if(pkg.mtd()==1)
                {
                    send_msg(my_data->Sd,pkg);
                }
                else
                {
                    myPkg sorted_item=pkg;
                    sorted_item.clear_allcip();
                    sorted_item.clear_allenc();
//                    vector<string> allcip;
                    vector<int64_t> allenc;
                    map<int64_t,string> tmp_map;
                    vector<int64_t> plain_list;
                    int all_size=pkg.allcip_size();
                    for(int i=0;i<all_size;i++)
                    {
                        string s1=pkg.allcip(i);
                        int64_t e1=pkg.allenc(i);
                        int64_t plain=text[s1];
                        tmp_map[plain]=s1;
                        plain_list.push_back(plain);
//                        allcip.push_back(s1);
                        allenc.push_back(e1);

                    }
                    sort(plain_list.begin(),plain_list.end());
                    sort(allenc.begin(),allenc.end());
                    assert(all_size==allenc.size());
                    for(int i=0;i<all_size;i++)
                    {
                        sorted_item.add_allcip(tmp_map[plain_list.at(i)]);
                        sorted_item.add_allenc(allenc[i]);
                    }
                    send_msg(my_data->Sd,sorted_item);
                }
            }
            else if(pkg.msgtype()==TX)
            {
                send_msg(my_data->Sd,pkg);
            }
            else if(pkg.msgtype()==SORT_UDZ)
            {
                myPkg sorted_item=pkg;
                sorted_item.set_msgtype(RESORT_TX);
//                vector<string> lcip;
                vector<int64_t> lenc;
                vector<string> rcip;
                vector<int64_t> renc;
                string path=sorted_item.path();
                char c=path[path.length()-1];
                if(c=='0')
                {
                    sorted_item.clear_lcip();
                    sorted_item.clear_lenc();
                    map<int64_t,string> tmp_map;
                    vector<int64_t> plain_list;
                    int lsize=pkg.lcip_size();
                    for(int i=0;i<lsize;i++)
                    {
                        string s1=pkg.lcip(i);
                        int64_t e1=pkg.lenc(i);
//                        lcip.push_back(s1);
                        int64_t plain=text[s1];
                        tmp_map[plain]=s1;
                        plain_list.push_back(plain);
                        lenc.push_back(e1);

                    }
//                    int l_size=lcip.size();
//                    assert(lsize==l_size);

                    //another sort
                    sort(plain_list.begin(),plain_list.end());
                    //sort end

                    sort(lenc.begin(),lenc.end());

                    for(int i=0;i<lsize;i++)
                    {
                        sorted_item.add_lcip(/*lcip[i]*/tmp_map[plain_list.at(i)]);
                        sorted_item.add_lenc(lenc[i]);
                    }
                    send_msg(my_data->Sd,sorted_item);
                }
                else
                {
                    sorted_item.clear_rcip();
                    sorted_item.clear_renc();
                    map<int64_t,string> tmp_map;
                    vector<int64_t> plain_list;
                    int rsize=pkg.rcip_size();
                    for(int i=0;i<rsize;i++)
                    {
                        string s2=pkg.rcip(i);
                        int64_t e2=pkg.renc(i);
//                        rcip.push_back(s2);
                        int64_t plain=text[s2];
                        tmp_map[plain]=s2;
                        plain_list.push_back(plain);
                        renc.push_back(e2);
                    }
//                    int r_size=rcip.size();
//                    assert(rsize==r_size);
                    //sort [cipher2,cipher1,cipher10,...]
                    //1.get plain_list[2,1,10,...], tmp_map(2,cipher2) (1,cipher1) ...
                    //2.sort plain_list [1,2,10,...]
                    //3.add rcip [cipher1,cipher2,cipher10,...]

                    sort(plain_list.begin(),plain_list.end());
                    //sort end

                    sort(renc.begin(),renc.end());

                    for(int i=0;i<rsize;i++)
                    {
                        sorted_item.add_rcip(/*rcip[i]*/tmp_map[plain_list.at(i)]);
                        sorted_item.add_renc(renc[i]);
                    }
                    send_msg(my_data->Sd,sorted_item);
                }
            }
            else
            {
                cerr<<"wrong message type when rec: "<<pkg.msgtype()<<endl;
            }

        }
    }
}
void *do_query(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    timeval start,end;
    sleep(12);
    myPkg pkg;
    pkg.set_msgtype(REQ_QRY);
    pkg.add_lcip("cipher500");
    pkg.add_rcip("cipher800");
//    pkg.add_lenc(480000000000);
//    pkg.add_renc(550000000000);
    send_msg(my_data->Sd,pkg);
    gettimeofday(&start,NULL);
    while (1) {
        if(query_pkg.msgtype()==QRY_END)
        {
            gettimeofday(&end,NULL);
            float time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
            int size=query_pkg.allcip_size();
            cout<<"result set size="<<size<<", latency="<<time_use<<endl;
            for(int i=0;i<size;i++)
                cout<<"result "<<query_pkg.allcip(i)<<","<<query_pkg.allenc(i)<<endl;
            break;
        }
    }
}
int main(int argc, char *argv[])
{
//    AES aes;
//    aes.initKV();
//    int plain=123;
//    string cipher=aes.encrypt(to_string(plain));
//    cout<<cipher<<endl;
//    string plainstr=aes.decrypt(cipher);
//    int de=std::stoi(plainstr);
//    cout<<de<<endl;
    text.insert(pair<string,int64_t>("cipher-1",-1));
    text.insert(pair<string,int64_t>("cipher"+to_string(PLSIZE),M));
    for(int64_t i=0;i<PLSIZE;i++)
        text.insert(pair<string,int64_t>("cipher"+to_string(i),i));
    char *serverIp = "127.0.0.1"; int port =8080/* atoi(argv[2])*/;
    //create a message buffer

    //setup a socket and connection tools
    struct hostent* host = gethostbyname(serverIp);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
            inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;return -1;
    }
    cout << "Connected to the server!" << endl;
    pthread_t tid3[NUM_REC];
    struct thread_data td3[NUM_REC];
    for(int i=0; i < NUM_REC; i++ ){
        td3[i].thread_id = i;
        td3[i].Sd = clientSd;
        int rc = pthread_create(&tid3[i], NULL,do_rec, (void *)&td3[i]);
        pthread_detach(tid3[i]);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
//    pthread_t tid4;
//    struct thread_data td4;
//    td4.Sd=clientSd;
//    int rc = pthread_create(&tid4, nullptr,do_query, (void *)&td4);
//    pthread_detach(tid4);
//    if (rc){
//        cout << "Error:unable to create thread," << rc << endl;
//        exit(-1);
//    }
    while(1)
    {

    }
    close(clientSd);
    pthread_exit(NULL);
    return 0;
}
