#ifndef UTIL_H
#define UTIL_H
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <jsoncpp/json.h>
#include "msg.pb.h"
using namespace std;
#define TXN 65536
//#define M 549755813888
extern int64_t M;
extern int64_t ERR;
//1073741824
#define BUFF_SIZE 10000
//#define ERR -9999
#define UDZ_SIZE 200
#define BLOCK_SIZE 50000
#define TX_PER_BLOCK 1000
using namespace std;
enum MSG_TYPE{
    REQ_CMP=1,
    CMP_RST,
    SORT_UDZ,
    RESORT_TX,
    TX,
    REB_TX,
    REQ_QRY,
    REQ_CMP_QRY,
    REQ_SORT_QRY,
    QRY_CMP_RST,
    QRY_SORT_RST,
    QRY_END
};
struct Node;
typedef struct UDZ
{
   std::vector<Node> list;
}UDZ;
typedef struct Node
{
   string cipher="";
   int64_t code=ERR;
    Node* left;
    Node* right;
    Node* parent;
//    int v=0;
    UDZ lz;
    UDZ rz;
    bool lock=0;
    Node(string x,int64_t y)
    {
        left=nullptr;
        right=nullptr;
        parent=nullptr;
        cipher=x;
        code=y;
    }
    Node()
    {
        left=nullptr;
        right=nullptr;
        parent=nullptr;
        code=ERR;

    }
    bool operator == (const Node & obj) const
    {
        return code == obj.code;
    }
    int get_lzise()
    {
        return lz.list.size();
    }
    int get_rzise()
    {
        return rz.list.size();
    }
    void push_l(Node node)
    {
        lz.list.push_back(node);
    }
    void push_r(Node node)
    {
        rz.list.push_back(node);
    }
}Node;
typedef struct Tx
{
    int tx_type=-1;
    string cipher="";
    int64_t y=ERR;
    string path="";
    int v_bef=0;
    //add lqq
    int v_udz=0;
    bool mtd=0;
//    int v_parent=0;
    //add lqq
    UDZ lz;
    UDZ rz;
    void Print()
    {
        cout<<"-----------------print tx----------------"<<endl;
        cout<<"tx_type:"<<tx_type<<endl;
        cout<<"cipher:"<<cipher<<endl;
        cout<<"y:"<<y<<endl;
        cout<<"path:"<<path<<endl;
        cout<<"v_bef:"<<v_bef<<endl;
//        cout<<"method:"<<mtd<<endl;
        if(tx_type==RESORT_TX)
        {
            cout<<"print left udz"<<endl;
            for(int i=0;i<lz.list.size();i++)
            cout<<"Node:"<<lz.list.at(i).cipher<<","<<lz.list.at(i).code<<endl;
             cout<<"print right udz"<<endl;
             for(int i=0;i<rz.list.size();i++)
             cout<<"Node:"<<rz.list.at(i).cipher<<","<<rz.list.at(i).code<<endl;
        }
        cout<<"-----------------print end----------------"<<endl;
    }
}Tx;
typedef struct Block
{
    vector<Tx> tx_batch;
    int n=0;
    int block_id=0;
    void push(Tx tx)
    {
        tx_batch.push_back(tx);
        n++;
    }
    Tx get(int i)
    {
        return tx_batch.at(i);
    }
    void clear()
    {
        tx_batch.clear();
        block_id=0;
        n=0;
    }

    void Print()
    {
        cout<<"--------print block-------"<<block_id<<endl;
        cout<<"block num is "<<n<<endl;
//        if(n==1){
//                for(int i=0;i<n;i++)
//                {
//                    Tx tx=tx_batch[i];
//                    tx.Print();
//                }
//        }
//        for(int i=0;i<n;i++)
//        {
//            Tx tx=tx_batch[i];
//            tx.Print();
//        }
//        cout<<"end is:"<<end<<endl;
        cout<<"--------end-------"<<endl;
    }
}Block;
typedef struct enc_req
{
    string cipher="";
    int id=0;
}enc_req;
struct thread_data{
   int thread_id;
   int Sd;
   string message;
   int left;
   int right;
};
//int send_msg(int Sd,send_item& item);
//int rec_msg(int Sd,send_item& item);
int send_msg(int Sd,myPkg& pkg);
int rec_msg(int Sd,myPkg& pkg);
int send_block(int Sd,Block& block);
int rec_block(int Sd,Block& block);
const char* serialize_block(Block& block);
Block deserialize_block(const char* buf);
#endif // UTIL_H
