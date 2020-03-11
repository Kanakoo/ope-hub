#ifndef UTIL_H
#define UTIL_H
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
//#include "json/json.h"
#include "./trans/msg.pb.h"
using namespace std;
#define PLSIZE /*131072*/65536 //plaintext domain
extern int64_t M;//ciphertext domain
extern int64_t ERR;
//1073741824
#define BUFF_SIZE 10000
//#define ERR -9999
#define UDZ_SIZE 200
#define B_SIZE 50000
#define TX_PER_BLOCK 400
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
//   Node* parent;
   UDZ()
   {
       //parent=nullptr;
   }
   UDZ& operator=(const UDZ& udz)
   {
       // 避免自赋值
       this->list.clear();
       for(int i=0;i<udz.list.size();i++)
       {
           this->list.push_back(udz.list[i]);
       }

//       this->parent=udz.parent;
       return *this;
   }
}UDZ;
typedef struct Node
{
   string cipher="";
   int64_t code=ERR;
    Node* left;
    Node* right;
    Node* parent;
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
        return (code == obj.code);
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
    //add lqq
    UDZ lz;
    UDZ rz;
    Tx()
    {
    }
    void Print()
    {
        cout<<"-----------------print tx----------------"<<endl;
        cout<<"tx_type:"<<tx_type<<endl;
        cout<<"cipher:"<<cipher<<endl;
        cout<<"y:"<<y<<endl;
        cout<<"path:"<<path<<endl;
        cout<<"v_bef:"<<v_bef<<endl;
        cout<<"-----------------print end----------------"<<endl;
    }
}Tx;
typedef struct Block
{
    vector<Tx> tx_batch;
    int n=0;
    int block_id=0;
    vector<int> lns;
    vector<int> rns;
    void push(Tx tx)
    {
        tx_batch.push_back(tx);
        lns.push_back(tx.lz.list.size());
        rns.push_back(tx.rz.list.size());
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
        lns.clear();
        rns.clear();
    }
    void serialize(char* buffer)
    {
       int pos=0;
       memcpy(buffer+pos,&n,sizeof(int));
       pos+=sizeof(n);
       memcpy(buffer+pos,&block_id,sizeof(int));
       pos+=sizeof(block_id);
       for(int i=0;i<n;i++)
       {
           memcpy(buffer+pos,&lns[i],sizeof(int));
           pos+=sizeof(lns[i]);
           memcpy(buffer+pos,&rns[i],sizeof(int));
           pos+=sizeof(rns[i]);
       }
       for(int i=0;i<n;i++)
       {
           Tx tx=tx_batch[i];
           memcpy(buffer+pos,&tx.tx_type,sizeof(int));
           pos+=sizeof(tx.tx_type);

           int cip_len=tx.cipher.length();
           memcpy(buffer+pos,&cip_len,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,tx.cipher.c_str(),cip_len);
           pos+=cip_len;

           memcpy(buffer+pos,&tx.y,sizeof(int64_t));
           pos+=sizeof(tx.y);

           int pa_len=tx.path.length();
           memcpy(buffer+pos,&pa_len,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,tx.path.c_str(),pa_len);
           pos+=pa_len;

           memcpy(buffer+pos,&tx.v_bef,sizeof(int));
           pos+=sizeof(tx.v_bef);

           memcpy(buffer+pos,&tx.v_udz,sizeof(int));
           pos+=sizeof(tx.v_udz);

//           memcpy(buffer+pos,&tx.mtd,sizeof(bool));
//           pos+=sizeof(tx.mtd);

           int l_size=lns[i];
           for(int j=0;j<l_size;j++)
           {
               int lcip_len=tx.lz.list[j].cipher.length();
               memcpy(buffer+pos,&lcip_len,sizeof(int));
               pos+=sizeof(int);
               memcpy(buffer+pos,tx.lz.list[j].cipher.c_str(),lcip_len);
               pos+=lcip_len;

               memcpy(buffer+pos,&tx.lz.list[j].code,sizeof(int64_t));
               pos+=sizeof(tx.lz.list[j].code);
           }
           int r_size=rns[i];
           for(int j=0;j<r_size;j++)
           {
               int rcip_len=tx.rz.list[j].cipher.length();
               memcpy(buffer+pos,&rcip_len,sizeof(int));
               pos+=sizeof(int);
               memcpy(buffer+pos,tx.rz.list[j].cipher.c_str(),rcip_len);
               pos+=rcip_len;

               memcpy(buffer+pos,&tx.rz.list[j].code,sizeof(int64_t));
               pos+=sizeof(tx.rz.list[j].code);
           }

       }
    }

    void deserialize(const char* buffer)
    {
       //cout<<"deserialize"<<endl;
        int pos=0;
        int *current=0;

        current = (int*)(buffer+pos);
        n=*current;
        pos+=sizeof(n);
        current = (int*)(buffer+pos);
        block_id=*current;
        pos+=sizeof(block_id);
        for(int i=0;i<n;i++)
        {
            current = (int*)(buffer+pos);
            lns.push_back(*current);
            pos+=sizeof(lns[i]);
            current = (int*)(buffer+pos);
            rns.push_back(*current);
            pos+=sizeof(rns[i]);
        }
        for(int i=0;i<n;i++)
        {
            Tx tx;
            current = (int*)(buffer+pos);
            tx.tx_type=*current;

            pos+=sizeof(tx.tx_type);

            current=(int*)(buffer+pos);
            int cip_len=*current;
            pos+=sizeof(int);
            string cipstr((char*)(buffer+pos),cip_len);
            tx.cipher=cipstr;
            pos+=cip_len;

            int64_t* curr;
            curr = (int64_t*)(buffer+pos);
            tx.y=*curr;
            pos+=sizeof(tx.y);

            current=(int*)(buffer+pos);
            int pa_len=*current;
            pos+=sizeof(int);
            string pastr((char*)(buffer+pos),pa_len);
            tx.path=pastr;
            pos+=pa_len;

            current = (int*)(buffer+pos);
            tx.v_bef=*current;
            pos+=sizeof(tx.v_bef);

            current = (int*)(buffer+pos);
            tx.v_udz=*current;
            pos+=sizeof(tx.v_udz);

//            bool* curr_bool = (bool*)(buffer+pos);
//            tx.mtd=*curr_bool;
//            pos+=sizeof(tx.mtd);

            int l_size=lns[i];
            for(int j=0;j<l_size;j++)
            {
                current=(int*)(buffer+pos);
                int lcip_len=*current;
                pos+=sizeof(int);
                string lcipstr((char*)(buffer+pos),lcip_len);
                pos+=lcip_len;

                curr = (int64_t*)(buffer+pos);
                int64_t lcode=*curr;
                pos+=sizeof(lcode);
                tx.lz.list.push_back(Node(lcipstr,lcode));
            }
            int r_size=rns[i];
            for(int j=0;j<r_size;j++)
            {
                current=(int*)(buffer+pos);
                int rcip_len=*current;
                pos+=sizeof(int);
                string rcipstr((char*)(buffer+pos),rcip_len);
                pos+=rcip_len;

                curr = (int64_t*)(buffer+pos);
                int64_t rcode=*curr;
                pos+=sizeof(rcode);
                tx.rz.list.push_back(Node(rcipstr,rcode));
            }
            tx_batch.push_back(tx);

        }
    }
    int get_serialize_size()
    {
       int pos=0;
       pos+=sizeof(n);
       pos+=sizeof(block_id);
       for(int i=0;i<n;i++)
       {
           pos+=sizeof(lns[i]);
           pos+=sizeof(rns[i]);
       }
       for(int i=0;i<n;i++)
       {
           Tx tx=tx_batch[i];
           pos+=sizeof(tx.tx_type);
           pos+=sizeof(int);
           pos+=tx.cipher.length();
           pos+=sizeof(tx.y);
           pos+=sizeof(int);
           pos+=tx.path.length();
           pos+=sizeof(tx.v_bef);
           pos+=sizeof(tx.v_udz);
//           pos+=sizeof(tx.mtd);
           int l_size=lns[i];
           for(int j=0;j<l_size;j++)
           {
               pos+=sizeof(int);
               pos+=tx.lz.list[j].cipher.length();
               pos+=sizeof(int64_t);
           }
           int r_size=rns[i];
           for(int j=0;j<r_size;j++)
           {
               pos+=sizeof(int);
               pos+=tx.rz.list[j].cipher.length();
               pos+=sizeof(int64_t);
           }

       }
       return pos;
    }
    void Print()
    {
        cout<<"--------print block-------"<<n<<endl;
        cout<<"block id is "<<block_id<<endl;
        for(int i=0;i<n;i++)
        {
            Tx tx=tx_batch[i];
            cout<<"tx: type="<<tx_batch[i].tx_type<<", code="<<tx_batch[i].y<<", cip="<<tx_batch[i].cipher<<", path="<<tx_batch[i].path<<endl;
            for(int j=0;j<lns[i];j++)
                cout<<"tx: left="<<tx.lz.list[j].code<<","<<tx.lz.list[j].cipher<<endl;
            for(int k=0;k<rns[i];k++)
                cout<<"tx: right="<<tx.rz.list[k].code<<","<<tx.rz.list[k].cipher<<endl;
        }
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
#endif // UTIL_H
