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
#include <random>
#include "tcp-Server.h"
using namespace std;
//add lqq
//variables
std::atomic<bool> is_reb{0};
std::atomic<bool> is_enc{0};
std::atomic<bool> method{0};
std::atomic<int> version{0};
std::atomic<int> version_udz{0};
std::atomic<int> abort_num{0};
std::atomic<int> conflict_num{0};
std::atomic<int> reb_num{0};
std::atomic<int> update_time{0};
std::atomic<int> nn{0};
std::atomic<float> delay{0.0};
timeval qstart;
timeval qend;
Node* root=new Node();
//HZC
CTSL::HashMap<int,vector<myPkg>> rst_list;
CTSL::HashMap<int,vector<myPkg>> qry_rst;
moodycamel::ConcurrentQueue<myPkg> query_pkgs;
std::atomic<int> block_id{0};
moodycamel::ConcurrentQueue<enc_req> enc_queue;
moodycamel::ConcurrentQueue<Tx> tx_buffer;
vector<int> re_sizes;
//CTSL::HashMap<int,vector<string>> test_list;
timeval mystart;
timeval myend;
int64_t dice64(int64_t begin, int64_t end)
{
    uint64_t range = end - begin;
    if (range <= 0) {
        // if invalid range, return begin
        return begin;
    }

    uint64_t selected = 0;
    uint64_t totalSize = 1000000000000000000LL;
    uint64_t divSize = 10000;
    uint64_t sectionSize = 0;

    // select random number between 0 ~ (1000000000000000000 - 1)
    while (true) {
        sectionSize = totalSize / divSize;
        selected += ((uint64_t)(rand() % divSize)) * sectionSize;
        totalSize = sectionSize;
        if (totalSize <= divSize) {
            selected += rand() % divSize;
            break;
        }
    }

    // selected mod range plus begin is randomNumber between begin ~ end
    return begin + (selected % range);
}
int64_t compute_rand_code(int64_t y1,int64_t y2)
{
    int64_t y=0;
    int64_t domain=(y2-y1)/3;
    y=y1+((y2-y1)/2);
    int64_t rand_n=dice64(-domain,domain);
    y=y+rand_n;
    return y;
}
int compute_one(bool is_l,string cipherx,Node* parent,myPkg& pkg)
{
    int64_t y=0,y1=0,y2=0;
    if(is_l)
    {
        y2=parent->code;
        Node* p=parent;
        while(p!=nullptr && p->code>=y2)
        {
            p=p->parent;
        }
        if(p!=nullptr)
            y1=p->code;
        y=compute_rand_code(y1,y2);
//        cout<<"y1="<<y1<<",y="<<y<<",y2="<<y2<<endl;
        int size=pkg.lenc_size();
        bool is_contain=0;
        for(int k=0;k<size;k++)
        {
            if(y==pkg.lenc(k))
            {
                is_contain=1;
                break;
            }
        }
        if(!is_contain)
        {
//            cout<<"add "<<cipherx<<","<<y<<endl;
            pkg.add_lcip(cipherx);
            pkg.add_lenc(y);
        }
        else
        {
            cout<<"abort "<<cipherx<<endl;
        }
    }
    else
    {
        y1=parent->code;
        Node* p=parent;
        while(p!=nullptr && p->code<=y1)
        {
            p=p->parent;
        }
        if(p!=nullptr)
            y2=p->code;
        y=compute_rand_code(y1,y2);
//        cout<<"y1="<<y1<<",y="<<y<<",y2="<<y2<<endl;
        int size=pkg.renc_size();
        bool is_contain=0;
        for(int k=0;k<size;k++)
        {
            if(y==pkg.renc(k))
            {
                is_contain=1;
                break;
            }
        }
        if(!is_contain)
        {
//            cout<<"add "<<cipherx<<","<<y<<endl;
            pkg.add_rcip(cipherx);
            pkg.add_renc(y);
        }
        else
        {
            cout<<"abort "<<cipherx<<endl;
        }
    }

}
//tree functions
void quicksort(vector<string>& tmp_list,map<string,int64_t>& tmp_map,int low, int high)
{
    if (low < high)
    {
        int l = low;
        int r = high;
        string key = tmp_list[l];//记录key值
        while (l < r)
        {
            while (l < r&& tmp_map[key] <= tmp_map[tmp_list.at(r)])//从右往左遍历,找到第一个小于key的元素
                --r;
            tmp_list[l] = tmp_list[r];
            while (l < r&& tmp_map[key] >= tmp_map[tmp_list.at(l)])//从左往右遍历,找到第一个大于key值的元素
                ++l;
            tmp_list[r] = tmp_list[l];
        }
        tmp_list[l] = key;//其实此时l=r

        quicksort(tmp_list, tmp_map, low, l-1);
        quicksort(tmp_list, tmp_map, r + 1, high);
    }
}
//for update
void Inorder(Node* node,vector<string>& tmp_list,map<string,int64_t>& tmp_map)
{
    if(node==nullptr)
        return;
    Inorder(node->left,tmp_list,tmp_map);
    if(node->code!=-1 && node->code!=M && node->code!=ERR)
    {
        tmp_list.push_back(node->cipher);
        tmp_map[node->cipher]=node->code;
    }
    Inorder(node->right,tmp_list,tmp_map);
}
void Inorder(vector<string>& tmp_list,map<string,int64_t>& tmp_map, vector<Node>& all_udz)
{
    tmp_map[root->cipher]=root->code;
    tmp_map[root->right->cipher]=root->right->code;
    for(int i=0;i<all_udz.size();i++)
    {
        string cip=all_udz[i].cipher;
        int64_t code=all_udz[i].code;
        tmp_map[cip]=code;
        tmp_list.push_back(cip);
    }
    Inorder(root,tmp_list,tmp_map);
}
//for print
void get_childnodes(Node* node,int& num)
{
    if(node==nullptr)
        return;
    get_childnodes(node->left,num);
    if(node->get_lzise()>0)
    {
        num+=node->get_lzise();
    }
    if(node->get_rzise()>0)
    {
        num+=node->get_rzise();
    }
    num++;
    get_childnodes(node->right,num);

}
void Inorder1(Node* node,int& num)
{
    if(node==nullptr)
        return;
    Inorder1(node->left,num);

    if(node->get_lzise()>0)
    {
        num+=node->get_lzise();
//        cout<<"------left--------of "<<node->cipher<<endl;
//        for(int i=0;i<node->get_lzise();i++)
//        {
//            Node tmp1=node->lz.list[i];
//            cout<<"entry: "<<"x="<<tmp1.cipher<<",y="<<tmp1.code<<endl;

//        }
//        cout<<"-------end-------"<<endl;
    }
    if(node->get_rzise()>0)
    {
        num+=node->get_rzise();
//        cout<<"------right--------of "<<node->cipher<<endl;
//        for(int i=0;i<node->get_rzise();i++)
//        {
//            Node tmp2=node->rz.list[i];
//            cout<<"entry: "<<"x="<<tmp2.cipher<<",y="<<tmp2.code<<endl;

//        }
//        cout<<"-------end-------"<<endl;
    }
    //cout<<"entry: "<<"x="<<node->cipher<<",y="<<node->code<<endl;
    num++;
    Inorder1(node->right,num);
}
void Print(int& num)
{
    num=0;
//    cout<<"====begin print===="<<endl;
    Inorder1(root,num);
    num-=2;
//    cout<<"====end print===="<<endl;
}
//for get all udz
void Inorder2(Node* node,myPkg& item)
{
    if(node==nullptr)
        return;
    Inorder2(node->left,item);
    int l_size=node->get_lzise(),r_size=node->get_rzise();
    if(l_size>0)
    {
        vector<Node> llist=node->lz.list;
        for(int i=0;i<l_size;i++)
        {
            item.add_allcip(llist[i].cipher);
            item.add_allenc(llist[i].code);
        }
    }
    if(r_size>0)
    {
        vector<Node> rlist=node->rz.list;
        for(int i=0;i<r_size;i++)
        {
            item.add_allcip(rlist[i].cipher);
            item.add_allenc(rlist[i].code);
        }
    }
    Inorder2(node->right,item);
}
void get_all_udz(myPkg& item)
{
    Inorder2(root,item);
}
int getHeight(Node* root)
{
    if(root==nullptr)
            return 0;
    int left_depth=getHeight(root->left)+1;
    int right_depth=getHeight(root->right)+1;
    return left_depth>right_depth?left_depth:right_depth;
}
Node* CreateBST(int low,int high,vector<Node> A)  //用关键字序列创建高度最小的二叉排序树,low,high初值为序列中最小数与最大数下标
{                           //这里的关键字序列为int数组，为节省内存使用全局变量
    int mid = (low+high)/2;
    Node *root=new Node(A[mid].cipher,A[mid].code);;
    if(low<=high)
    {
        Node* leftchild=CreateBST(low,mid-1,A);
        root->left=leftchild;
        if(leftchild!=nullptr)
            leftchild->parent=root;
        Node* rightchild=CreateBST(mid+1,high,A);
        root->right=rightchild;
        if(rightchild!=nullptr)
            rightchild->parent=root;
        return root;
    }
    else
        return NULL;
}
void destory(Node*& p)
{
    if(p!=nullptr)
    {
        if(p->left!=nullptr)
            destory(p->left);
        if(p->right!=nullptr)
            destory(p->right);
        delete p;
        p=nullptr;
    }
}
void destory()
{
    destory(root);
}
void traverseByCode(Node* root,int64_t k1,int64_t k2,myPkg& pkg)
{//采用前序遍历
    if(root==nullptr)
        return;
    if(k1 <= root->code && k2 >= root->code)//满足条件的就存入
    {
        pkg.add_allcip(root->cipher);
        pkg.add_allenc(root->code);
    }
    traverseByCode(root->left,k1,k2,pkg);
    traverseByCode(root->right,k1,k2,pkg);
}
void searchRangeByCode(int64_t k1,int64_t k2, myPkg& pkg)
{
    // write your code here
    if(root != nullptr && root->right!=nullptr)
        traverseByCode(root->right,k1,k2,pkg);
}
void searchRangeByCipher(int Sd,string c1,string c2,myPkg& qry_pkg)
{
    if(root != nullptr && root->right!=nullptr)
    {
        Node* tmp=root->right;
        Node* parent=nullptr;
        int msg_id=0;
        int64_t cmp=0;
        int64_t k1=0,k2=0;
        //find k1
        while(1)
        {
            if(tmp==nullptr)
            {
                k1=parent->code;
                break;
            }
            parent=tmp;
            myPkg pkg;
            pkg.set_encid(qry_pkg.encid());
            pkg.set_msgtype(REQ_CMP_QRY);
            pkg.set_msgid(msg_id);
            pkg.set_tmp(tmp->cipher);
            pkg.set_cipher(c1);
            send_msg(Sd,pkg);
            while(1)
            {
                vector<myPkg> tmpv;
                if(qry_rst.find(pkg.encid(),tmpv) && tmpv.size()>msg_id)
                {
                    myPkg rec_rst1=tmpv.at(msg_id);
                    msg_id++;
                    cmp=rec_rst1.rst();
                    break;
                }
            }
            if(cmp==0)
            {
                k1= tmp->code;
                break;
            }
            else if(cmp>0)
                tmp=tmp->left;
            else
                tmp=tmp->right;
        }
        //find k2
        Node* tmp1=root->right;
        Node* parent1=nullptr;
        while(1)
        {
            if(tmp1==nullptr)
            {
                k2=parent1->code;
                break;
            }
            parent1=tmp1;
            myPkg pkg;
            pkg.set_encid(qry_pkg.encid());
            pkg.set_msgtype(REQ_CMP_QRY);
            pkg.set_msgid(msg_id);
            pkg.set_tmp(tmp1->cipher);
            pkg.set_cipher(c2);
            send_msg(Sd,pkg);
            while(1)
            {
                vector<myPkg> tmpv;
                if(qry_rst.find(pkg.encid(),tmpv) && tmpv.size()>msg_id)
                {
                    myPkg rec_rst1=tmpv.at(msg_id);
                    msg_id++;
                    cmp=rec_rst1.rst();
                    break;
                }
            }
            if(cmp==0)
            {
                k2= tmp1->code;
                break;
            }
            else if(cmp>0)
                tmp1=tmp1->left;
            else
                tmp1=tmp1->right;
        }
        searchRangeByCode(k1,k2,qry_pkg);
    }
}

void searchOneByCipherUDZ(int Sd,string c,int64_t& k,Node*& p,int& msg_id,int qry_id)
{
    if(root != nullptr && root->right!=nullptr)
    {
        Node* tmp=root->right;
        Node* parent=nullptr;
        int64_t cmp=0;
        while(1)
        {
            if(tmp==nullptr)
            {
                k=parent->code;
                break;
            }
            parent=tmp;
            myPkg pkg;
            pkg.set_encid(qry_id);
            pkg.set_msgtype(REQ_CMP_QRY);
            pkg.set_msgid(msg_id);
            pkg.set_tmp(tmp->cipher);
            pkg.set_cipher(c);
            send_msg(Sd,pkg);
            while(1)
            {
                vector<myPkg> tmpv;
                if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id)
                {
                    myPkg rec_rst1=tmpv.at(msg_id);
                    msg_id++;
                    cmp=rec_rst1.rst();
                    break;
                }
            }
            if(cmp==0)
            {
                k= tmp->code;
                break;
            }
            else if(cmp>0)
                tmp=tmp->left;
            else
                tmp=tmp->right;
        }
        p=parent;
        if(parent!=nullptr && cmp!=0)
        {
            k=parent->code;
            if(cmp>0)
            {
                int l_size=parent->get_lzise();
                if(l_size>0)
                {
                    myPkg udz_item;
                    udz_item.set_encid(qry_id);
                    udz_item.set_rst(cmp);
                    udz_item.set_msgtype(REQ_SORT_QRY);
                    for(int j=0;j<l_size;j++)
                    {
                        udz_item.add_lcip(parent->lz.list[j].cipher);
                        udz_item.add_lenc(parent->lz.list[j].code);
                    }
                    send_msg(Sd,udz_item);
                    while (1) {
                        vector<myPkg> tmpv;
                        if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
                        {
                            myPkg sorted_item=tmpv.at(msg_id);
                            msg_id++;
                            vector<Node> llist;
                            int lcount=sorted_item.lcip_size();
                            k=sorted_item.lenc(lcount/2);
                            for(int i=0;i<lcount;i++)
                            {
                                string cip=sorted_item.lcip(i);
                                int64_t code=sorted_item.lenc(i);
                                llist.push_back(Node(cip,code));
                                if(cip==c)
                                    k=code;
                            }
                            parent->lz.list.clear();
                            Node* childtree=CreateBST(0,lcount-1,llist);
                            parent->left=childtree;
                            childtree->parent=parent;
                            break;
                        }
                    }
                }
            }
            else
            {
                int r_size=parent->get_rzise();
                if(r_size>0)
                {
                    myPkg udz_item;
                    udz_item.set_encid(qry_id);
                    udz_item.set_rst(cmp);
                    udz_item.set_msgtype(REQ_SORT_QRY);
                    for(int j=0;j<r_size;j++)
                    {
                        udz_item.add_rcip(parent->rz.list[j].cipher);
                        udz_item.add_renc(parent->rz.list[j].code);
                    }
                    send_msg(Sd,udz_item);
                    while (1) {
//                        myPkg sorted_item;
//                        bool found=qry_rst.try_dequeue(sorted_item);
//                        if(found && sorted_item.msgtype()==QRY_SORT_RST)
//                        {
                        vector<myPkg> tmpv;
                        if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
                        {
                            myPkg sorted_item=tmpv.at(msg_id);
                            msg_id++;
                            vector<Node> rlist;
                            int rcount=sorted_item.rcip_size();
                            k=sorted_item.renc(rcount/2);
                            for(int i=0;i<rcount;i++)
                            {
                                string cip=sorted_item.rcip(i);
                                int64_t code=sorted_item.renc(i);
                                rlist.push_back(Node(cip,code));
                                if(cip==c)
                                    k=code;
                            }
                            parent->rz.list.clear();
                            Node* childtree=CreateBST(0,rcount-1,rlist);
                            parent->right=childtree;
                            childtree->parent=parent;
                            break;
                        }
                    }
                }
            }
        }

    }
}
void searchOneByCodeUDZ(int Sd,int64_t k,Node*& p,int& msg_id,int qry_id)
{
    if(root != nullptr && root->right!=nullptr)
    {
        Node* tmp=root->right;
        Node* parent=nullptr;
        int64_t cmp=0;
        while(1)
        {
            if(tmp==nullptr)
            {
                k=parent->code;
                break;
            }
            parent=tmp;
            if(tmp->code==k)
            {
                break;
            }
            else if(tmp->code>k)
            {
                cmp=0;
                tmp=tmp->left;
            }
            else
            {
                tmp=tmp->right;
                cmp=1;
            }
        }
        p=parent;
        if(parent!=nullptr && cmp!=0)
        {
            if(cmp>0)
            {
                int l_size=parent->get_lzise();
                if(l_size>0)
                {
                    myPkg udz_item;
                    udz_item.set_encid(qry_id);
                    udz_item.set_rst(cmp);
                    udz_item.set_msgtype(REQ_SORT_QRY);
                    for(int j=0;j<l_size;j++)
                    {
                        udz_item.add_lcip(parent->lz.list[j].cipher);
                        udz_item.add_lenc(parent->lz.list[j].code);
                    }
                    send_msg(Sd,udz_item);
                    while (1) {
//                        myPkg sorted_item;
//                        bool found=qry_rst.try_dequeue(sorted_item);
//                        if(found && sorted_item.msgtype()==QRY_SORT_RST)
//                        {
                        vector<myPkg> tmpv;
                        if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
                        {
                            myPkg sorted_item=tmpv.at(msg_id);
                            msg_id++;
                            vector<Node> llist;
                            int lcount=sorted_item.lcip_size();
                            k=sorted_item.lenc(lcount/2);
                            for(int i=0;i<lcount;i++)
                            {
                                string cip=sorted_item.lcip(i);
                                int64_t code=sorted_item.lenc(i);
                                llist.push_back(Node(cip,code));
                            }
                            parent->lz.list.clear();
                            Node* childtree=CreateBST(0,lcount-1,llist);
                            parent->left=childtree;
                            childtree->parent=parent;
                            break;
                        }
                    }

                }
            }
            else
            {
                int r_size=parent->get_rzise();
                if(r_size>0)
                {
                    myPkg udz_item;
                    udz_item.set_encid(qry_id);
                    udz_item.set_rst(cmp);
                    udz_item.set_msgtype(REQ_SORT_QRY);
                    for(int j=0;j<r_size;j++)
                    {
                        udz_item.add_rcip(parent->rz.list[j].cipher);
                        udz_item.add_renc(parent->rz.list[j].code);
                    }
                    send_msg(Sd,udz_item);
                    while (1) {
//                        myPkg sorted_item;
//                        bool found=qry_rst.try_dequeue(sorted_item);
//                        if(found && sorted_item.msgtype()==QRY_SORT_RST)
//                        {
                        vector<myPkg> tmpv;
                        if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
                        {
                            myPkg sorted_item=tmpv.at(msg_id);
                            msg_id++;
                            vector<Node> rlist;
                            int rcount=sorted_item.rcip_size();
                            k=sorted_item.renc(rcount/2);
                            for(int i=0;i<rcount;i++)
                            {
                                string cip=sorted_item.rcip(i);
                                int64_t code=sorted_item.renc(i);
                                rlist.push_back(Node(cip,code));
                            }
                            parent->rz.list.clear();
                            Node* childtree=CreateBST(0,rcount-1,rlist);
                            parent->right=childtree;
                            childtree->parent=parent;
                            break;
                        }
                    }
                }
            }
        }

    }
}
void traverseByCodeUDZ(int Sd,Node *root,int& msg_id,int qry_id)
{
    if(root==nullptr)
        return;
    int l_size=root->get_lzise();
    if(l_size>0)
    {
        myPkg udz_item;
        udz_item.set_encid(qry_id);
        udz_item.set_rst(0);
        udz_item.set_msgtype(REQ_SORT_QRY);
        for(int j=0;j<l_size;j++)
        {
            udz_item.add_lcip(root->lz.list[j].cipher);
            udz_item.add_lenc(root->lz.list[j].code);
        }
        send_msg(Sd,udz_item);
        while (1) {
            vector<myPkg> tmpv;
            if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
            {
                myPkg sorted_item=tmpv.at(msg_id);
                msg_id++;
                vector<Node> llist;
                int lcount=sorted_item.lcip_size();
                for(int i=0;i<lcount;i++)
                {
                    string cip=sorted_item.lcip(i);
                    int64_t code=sorted_item.lenc(i);
                    llist.push_back(Node(cip,code));
                }
                root->lz.list.clear();
                Node* childtree=CreateBST(0,lcount-1,llist);
                root->left=childtree;
                childtree->parent=root;
                break;
            }
        }
    }
    int r_size=root->get_rzise();
    if(r_size>0)
    {
        myPkg udz_item;
        udz_item.set_encid(qry_id);
        udz_item.set_rst(1);
        udz_item.set_msgtype(REQ_SORT_QRY);
        for(int j=0;j<r_size;j++)
        {
            udz_item.add_rcip(root->rz.list[j].cipher);
            udz_item.add_renc(root->rz.list[j].code);
        }
        send_msg(Sd,udz_item);
        while (1) {
            vector<myPkg> tmpv;
            if(qry_rst.find(qry_id,tmpv) && tmpv.size()>msg_id && tmpv.at(msg_id).msgtype()==QRY_SORT_RST)
            {
                myPkg sorted_item=tmpv.at(msg_id);
                msg_id++;
                vector<Node> rlist;
                int rcount=sorted_item.rcip_size();
                for(int i=0;i<rcount;i++)
                {
                    string cip=sorted_item.rcip(i);
                    int64_t code=sorted_item.renc(i);
                    rlist.push_back(Node(cip,code));
                }
                root->rz.list.clear();
                Node* childtree=CreateBST(0,rcount-1,rlist);
                root->right=childtree;
                childtree->parent=root;
                break;
            }
        }
    }
    traverseByCodeUDZ(Sd,root->left,msg_id,qry_id);
    traverseByCodeUDZ(Sd,root->right,msg_id,qry_id);
}
Node* lowestCommonAncestor(Node* root, Node* p, Node* q) {
        if(root==nullptr) return 0;
        if(root->code>max(p->code,q->code)) return lowestCommonAncestor(root->left,p,q);
        if(root->code<min(p->code,q->code)) return lowestCommonAncestor(root->right,p,q);
        else return root;
    }
void searchRangeByCipherUDZ(int Sd, string c1, string c2,myPkg& pkg,int& msg_id)
{
    if(root != nullptr && root->right!=nullptr)
    {
        Node* left_range=nullptr;
        Node* right_range=nullptr;
        int64_t k1=0,k2=0;
        searchOneByCipherUDZ(Sd,c1,k1,left_range,msg_id,pkg.encid());
        searchOneByCipherUDZ(Sd,c2,k2,right_range,msg_id,pkg.encid());
        Node* parent=lowestCommonAncestor(root->right,left_range,right_range);
        traverseByCodeUDZ(Sd,parent,msg_id,pkg.encid());
        traverseByCode(parent,k1,k2,pkg);
    }
}
void searchRangeByCodeUDZ(int Sd,int64_t k1,int64_t k2,myPkg& pkg,int& msg_id)
{
    if(root != nullptr && root->right!=nullptr)
    {
        Node* left_range=nullptr;
        Node* right_range=nullptr;
        searchOneByCodeUDZ(Sd,k1,left_range,msg_id,pkg.encid());
        searchOneByCodeUDZ(Sd,k2,right_range,msg_id,pkg.encid());
        Node* parent=lowestCommonAncestor(root->right,left_range,right_range);
//        cout<<parent->cipher;
        traverseByCodeUDZ(Sd,parent,msg_id,pkg.encid());
        traverseByCode(parent,k1,k2,pkg);
    }
}

void execute_by_type(Tx& tx,Block& tx_block)
{
    //cout<<"--------------------tx method:"<<tx.mtd<<endl;
    if(tx.mtd==0)
        execute_tx_m0(tx,tx_block);
    else
        execute_tx_m1(tx,tx_block);
}
void execute_tx_m0(Tx& tx,Block& tx_block)
{

//    cout<<"*********execute by m0:"<<tx.cipher<<","<<tx.tx_type<<endl;
    if(tx.tx_type==REB_TX)
    {
        if(version_udz!=tx.v_udz)
        {
            return;
        }
        while(is_enc)
        {

        }
        is_reb=1;
        version++;
        version_udz++;
        vector <Node> sorted_udz=tx.lz.list;
        vector<string> tmp_list;
        map<string,int64_t> tmp_map;
        Inorder(tmp_list,tmp_map,sorted_udz);
        int len=tmp_list.size();
        quicksort(tmp_list,tmp_map,0,len-1);
        destory();
        root=new Node("cipher-1",-1);
        Node* node=new Node("cipher"+to_string(PLSIZE),M);
        root->right=node;
        node->parent=root;
        Update(tmp_list,tmp_map);
        is_reb=0;

    }
    else if(tx.tx_type==RESORT_TX)
    {
        int v_aft=version;
        if(v_aft!=tx.v_bef)
        {
            return;
        }
        else
        {
            Node* tmp=root;
            string path=tx.path;

            for(int i=0;i<path.length()-1;i++)
            {
                if(tmp!=nullptr)
                {
                    if(path[i]=='0')
                        tmp=tmp->left;
                    else
                        tmp=tmp->right;
                }
                else
                {
                    return;
                }
            }
            char c=path[path.length()-1];
            if(c=='0')
            {
                vector<Node> llist=tx.lz.list;
                tmp->lz.list.clear();
                int lcount=llist.size();
                Node* childtree=CreateBST(0,lcount-1,llist);
                tmp->left=childtree;
                childtree->parent=tmp;
                version_udz++;
            }
            else
            {
                assert(c=='1');
                vector<Node> rlist=tx.rz.list;
                tmp->rz.list.clear();
                int rcount=rlist.size();
                Node* childtree=CreateBST(0,rcount-1,rlist);
                tmp->right=childtree;
                childtree->parent=tmp;
                version_udz++;
            }
            tmp->lock=0;
        }
    }
    else if(tx.tx_type==TX)
    {
        int v_aft=version;
        if(v_aft!=tx.v_bef)
        {
            reb_num++;
            return;
        }
        else
        {
            Node* tmp=root;
            string path=tx.path;
            Node* parent=nullptr;
            for(int i=0;i<path.length()-1;i++)
            {
                if(tmp!=nullptr)
                {
                    parent=tmp;
                    if(path[i]=='0')
                        tmp=tmp->left;
                    else
                        tmp=tmp->right;
                }
            }
            Node* node=new Node(tx.cipher,tx.y);
            char c=path[path.length()-1];
            if(tmp==nullptr)
            {
                tmp=parent;
                c=path[path.length()-2];
            }
            if(c=='0')
            {
                if(tmp->get_lzise()>0 && tmp->get_lzise()<UDZ_SIZE)
                {
                    vector<Node> llist=tmp->lz.list;
                    vector<Node>::iterator iter;
                    Node n1("test",tx.y);
                    iter = find(llist.begin(), llist.end(), n1);
                    if (iter != llist.end())
                    {
                        return;
                    }
                    else
                    {
                        tmp->push_l(*node);
                        version_udz++;
                    }
                }
                else if(tmp->get_lzise()>0 && tmp->get_lzise()>=UDZ_SIZE)
                {
//                    cout<<"abort full:"<<tx.cipher<<endl;
                    return;
                }
                else if(tmp->get_lzise()<=0 && tmp->left!=nullptr)
                {
                    Node* e=tmp->left;
                    if(e->code==tx.y)
                    {
                        return;
                    }
                    else if(e->left!=nullptr || e->right!=nullptr)
                    {
                        update_time++;
//                        int num=0;
//                        get_childnodes(e,num);
//                        cout<<"!!!!!!!!!!:"<<num<<endl;
                        return;
                    }
                    else
                    {
                        tmp->push_l(*e);
                        tmp->left=nullptr;
                        tmp->push_l(*node);
                        version_udz++;
                    }
                }
                else if(tmp->get_lzise()<=0 && tmp->left==nullptr)
                {
                    node->parent=tmp;
                    tmp->left=node;
                }
                else
                {
//                    return;
                }

            }
            else
            {
                if(tmp->get_rzise()>0 && tmp->get_rzise()<UDZ_SIZE)
                {
                    vector<Node> rlist=tmp->rz.list;
                    vector<Node>::iterator iter;
                    Node n1("test",tx.y);
                    iter = find(rlist.begin(), rlist.end(), n1);
                    if (iter != rlist.end())
                    {
                        return;
                    }
                    else
                    {
                        tmp->push_r(*node);
                        version_udz++;
                    }

                }
                else if(tmp->get_rzise()>0 && tmp->get_rzise()>=UDZ_SIZE)
                {
//                    cout<<"abort full:"<<tx.cipher<<endl;
                    return;
                }
                else if(tmp->get_rzise()<=0 && tmp->right!=nullptr)
                {
                    Node* e=tmp->right;
                    if(e->code==tx.y)
                    {
                        return;
                    }
                    else if(e->left!=nullptr || e->right!=nullptr)
                    {
                        update_time++;
//                        int num=0;
//                        get_childnodes(e,num);
//                        cout<<"!!!!!!!!!!:"<<num<<endl;
                        return;
                    }
                    else
                    {
                        tmp->push_r(*e);
                        tmp->right=nullptr;
                        tmp->push_r(*node);
                        version_udz++;
                    }
                }
                else if(tmp->get_rzise()<=0 && tmp->right==nullptr)
                {
                    node->parent=tmp;
                    tmp->right=node;
                }
                else
                {
                    return;
                }

            }
        }
    }
    else
    {
        //cerr<<"wrong msg type:"<<tx.tx_type<<endl;
        return;
    }
    tx_block.push(tx);
}

int64_t Encode1(string current,map<string,int64_t>& tmp_map)
{
    Node* tmp=root;
    Node* parent=nullptr;
    int y=0,y1=0,y2=0;
    Node* node=new Node(current,ERR);
    if(tmp==nullptr)
    {
        return ERR;
    }
    else
    {
        while(tmp!=nullptr)
        {
            parent=tmp;
            if(tmp_map[current] == tmp_map[parent->cipher])
            {
                return tmp->code;
            }
            else if(tmp_map[current] < tmp_map[parent->cipher])
            {
                tmp=tmp->left;
            }
            else
            {
                tmp=tmp->right;
            }
        }
        if(parent!=nullptr)
        {
            node->parent=parent;
            if(tmp_map[current] < tmp_map[parent->cipher])
            {
                if(parent->left!=nullptr)
                {
                    return ERR;
                }
                parent->left=node;
                y2=parent->code;
                Node* p=parent;
                while(p!=nullptr && p->code>=y2)
                {
                    p=p->parent;
                }
                if(p!=nullptr)
                    y1=p->code;

            }
            else
            {
                if(parent->right!=nullptr)
                {
                    return ERR;
                }
                parent->right=node;
                y1=parent->code;
                Node* p=parent;
                while(p!=nullptr && p->code<=y1)
                {
                    p=p->parent;
                }
                if(p!=nullptr)
                    y2=p->code;
            }
            y=y1+ceil((float)(y2-y1)/2);
            node->code=y;
            return -2;
        }
        else
        {
            return ERR;
        }
    }
}
void Update(vector<string>& data,map<string,int64_t>& tmp_map)
{
    int i=data.size();//插入x之前有i个数需要重新编码
    if(i<=0)
    {
        cout<<"!!!!!!!!tree is nullptr!"<<endl;
        return;
    }
    int idx=floor((float)i/2)+1;//idx是中值的下标
    string current=data[idx-1];
    //cout<<"clear update "<<data[idx-1]<<endl;
    Encode1(current,tmp_map);//-1是因为相对于数组中的位置
    vector<string> left_lst;
    vector<string> right_lst;
    left_lst.assign(data.begin(), data.begin()+idx-1);
    right_lst.assign(data.begin()+idx, data.end());
    if(i>1) Update(left_lst,tmp_map);
    if(i>2) Update(right_lst,tmp_map);

}
int Encode_m0(int Sd)
{
    enc_req enc;
    bool found=enc_queue.try_dequeue(enc);
    if(found)
    {
//        cout<<"----------encode by m0:"<<enc.cipher<<endl;
        if(root==nullptr)
        {
            return ERR;
        }
        else if(root->right==nullptr)
        {
            return ERR;
        }
        else
        {
            nn++;
//            vector<string> mm(100,"");
//            test_list.insert(enc.id,mm);
            int64_t y1=0,y2=0,y=0;
            int v_bef=version;
            string path="1";
            string cipherx=enc.cipher;
            int enc_id=enc.id;
            int msg_id=0;
            Node* tmp=root->right;
            Node* parent=nullptr;
            while(1)
            {
                if(tmp==nullptr)
                    break;
                parent=tmp;
                myPkg pkg;
                pkg.set_msgtype(REQ_CMP);
                pkg.set_encid(enc_id);
                pkg.set_msgid(msg_id);
                pkg.set_tmp(tmp->cipher);
                pkg.set_cipher(cipherx);
                pkg.set_path(path);
                pkg.set_mtd(0);
                send_msg(Sd,pkg);
                while(1)
                {
                    vector<myPkg> tmpv;
                    if(rst_list.find(enc_id,tmpv) && tmpv.size()>msg_id)
                    {
                        myPkg rec_rst1=tmpv.at(msg_id);
                        msg_id++;
                        path=rec_rst1.path();
                        break;
                    }
                }
                if(path=="0")
                {
                    update_time++;
                    return tmp->code;
                }
                char next1=path[path.length()-1];
                if(next1=='0')
                {
                    tmp=tmp->left;
                }
                else
                {
                    tmp=tmp->right;
                }


            }
            if(parent!=nullptr)
            {
                while (parent->lock==1) {
                    usleep(1);
                }
                if(path[path.length()-1]=='0')
                {
                    //handle left udz
                    int l_size=parent->get_lzise();
                    //add com duplicate
                    if(l_size>0 && l_size<UDZ_SIZE)
                    {
                        for(int i=0;i<l_size;i++)
                        {
                            if(parent->lz.list[i].cipher==cipherx)
                            {
                                update_time++;
                                return 0;
                            }
                        }
                    }
                    //add end
                    if(l_size>=UDZ_SIZE)
                    {
                        parent->lock=1;
                        update_time++;
                        myPkg udz_item;
                        udz_item.set_vbef(v_bef);
                        udz_item.set_msgtype(SORT_UDZ);
                        udz_item.set_path(path);
                        udz_item.set_mtd(0);
                        for(int j=0;j<l_size;j++)
                        {
                            udz_item.add_lcip(parent->lz.list[j].cipher);
                            udz_item.add_lenc(parent->lz.list[j].code);
                        }
//                        compute_one(1,cipherx,parent,udz_item);
                        send_msg(Sd,udz_item);
                        return 0;
                    }
                    //handle end
                    y2=parent->code;
                    Node* p=parent;
                    while(p!=nullptr && p->code>=y2)
                    {
                        p=p->parent;
                    }
                    if(p!=nullptr)
                        y1=p->code;

                }
                else
                {
                    //handle right udz
                    int r_size=parent->get_rzise();
                    //add com duplicate
                    if(r_size>0 && r_size<UDZ_SIZE)
                    {
                        for(int i=0;i<r_size;i++)
                        {
                            if(parent->rz.list[i].cipher==cipherx)
                            {
                                update_time++;
                                return 0;
                            }
                        }
                    }
                    //add end
                    if(r_size>=UDZ_SIZE)
                    {
                        parent->lock=1;
                        update_time++;
                        myPkg udz_item;
                        udz_item.set_vbef(v_bef);
                        udz_item.set_msgtype(SORT_UDZ);
                        udz_item.set_path(path);
                        udz_item.set_mtd(0);
                        for(int j=0;j<r_size;j++)
                        {
                            udz_item.add_rcip(parent->rz.list[j].cipher);
                            udz_item.add_renc(parent->rz.list[j].code);
                        }
//                        compute_one(0,cipherx,parent,udz_item);
                        send_msg(Sd,udz_item);
                        return 0;
                    }
                    //handle end
                    y1=parent->code;
                    Node* p=parent;
                    while(p!=nullptr && p->code<=y1)
                    {
                        p=p->parent;
                    }
                    if(p!=nullptr)
                        y2=p->code;
                }
                myPkg tx_item;
                tx_item.set_mtd(0);
                //Tx tmp_tx;
                if(y2-y1==1)
                {
                    cout<<"--------------update m0:"<<cipherx<<endl;
                    update_time++;
                    tx_item.set_msgtype(REB_TX);
                    tx_item.set_vudz(version_udz);
                    get_all_udz(tx_item);
                    send_msg(Sd,tx_item);
                    return 0;
                }
                else
                {
                    y=compute_rand_code(y1,y2);
                    Tx tx;
                    tx.tx_type=TX;
                    tx.v_bef=v_bef;
                    tx.y=y;
                    tx.cipher=cipherx;
                    tx.path=path;
                    tx.mtd=0;
                    tx_buffer.enqueue(tx);
                    return 0;
                }
            }
            else
            {
                cout<<"!!!!err parent is nullptr"<<endl;
                return ERR;
            }
        }
    }
}
int Encode_m1(int Sd)
{

    enc_req enc;
    bool found=enc_queue.try_dequeue(enc);
    if(found)
    {
        //cout<<"-----------------------------------------------------------start old"<<endl;
        if(root==nullptr)
            return ERR;
        else if(root->right==nullptr)
        {
            return ERR;
        }
        else
        {
//            vector<string> mm(100,"");
//            test_list.insert(enc.id,mm);
            int64_t y1=0,y2=0,y=0;
            int v_bef=version;
            string path="1";
            string cipherx=enc.cipher;
            int enc_id=enc.id;
            int msg_id=0;
            Node* tmp=root->right;
            Node* parent=nullptr;
            while(1)
            {
                if(tmp==nullptr)
                    break;
                parent=tmp;
                myPkg pkg;
                pkg.set_msgtype(REQ_CMP);
                pkg.set_encid(enc_id);
                pkg.set_msgid(msg_id);
                pkg.set_tmp(tmp->cipher);
                pkg.set_cipher(cipherx);
                pkg.set_path(path);
                pkg.set_mtd(1);
                send_msg(Sd,pkg);
                while(1)
                {
                    vector<myPkg> tmpv;
                    if(rst_list.find(enc_id,tmpv) && tmpv.size()>msg_id)
                    {
                        myPkg rec_rst1=tmpv.at(msg_id);
                        msg_id++;
                        path=rec_rst1.path();
                        break;
                    }
//                    vector<string> tmpv;
//                    if(test_list.find(enc_id,tmpv) && tmpv.at(msg_id)!="")
//                    {
//                        path=tmpv[msg_id];
//                        msg_id++;
//                        break;
//                    }
                }
                if(path=="0")
                {
                    update_time++;
                    return tmp->code;
                }
                char next1=path[path.length()-1];
                if(next1=='0')
                {
//                    cout<<cipherx<<"<"<<tmp->cipher<<endl;
                    tmp=tmp->left;
                }
                else
                {
//                    cout<<cipherx<<">"<<tmp->cipher<<endl;
                    tmp=tmp->right;
                }


            }
            if(parent!=nullptr)
            {
                if(path[path.length()-1]=='0')
                {
                    y2=parent->code;
                    Node* p=parent;
                    while(p!=nullptr && p->code>=y2)
                    {
                        p=p->parent;
                    }
                    if(p!=nullptr)
                        y1=p->code;

                }
                else
                {
                    y1=parent->code;
                    Node* p=parent;
                    while(p!=nullptr && p->code<=y1)
                    {
                        p=p->parent;
                    }
                    if(p!=nullptr)
                        y2=p->code;
                }

//                Tx tx;
                myPkg tx_item;
                if(y2-y1==1)
                {
                    cout<<"--------------update m1:"<<cipherx<<endl;
                    update_time++;
//                    tx.tx_type=REB_TX;
                    tx_item.set_msgtype(REB_TX);
                }
                else
                {
//                    tx.tx_type=TX;
                    tx_item.set_msgtype(TX);
                    y=y1+((y2-y1)/2);
                }

//                tx.v_bef=v_bef;
//                tx.y=y;
//                tx.cipher=cipherx;
//                tx.path=path;
//                tx.mtd=1;
                tx_item.set_vbef(v_bef);
                tx_item.set_y(y);
                tx_item.set_cipher(cipherx);
                tx_item.set_path(path);
                tx_item.set_mtd(1);
                send_msg(Sd,tx_item);
//                tx_buffer.enqueue(tx);
                return 0;

            }
            else
            {
                cout<<"!!!!err parent is nullptr"<<endl;
                return ERR;
            }

        }
    }

}
void execute_tx_m1(Tx& tx,Block& tx_block)
{
//    cout<<"*********execute by m1:"<<tx.cipher<<","<<tx.tx_type<<endl;
    if(tx.tx_type==REB_TX)
    {
        while(is_enc)
        {

        }
        is_reb=1;
        version++;
        vector<string> tmp_list;
        map<string,int64_t> tmp_map;
        Inorder4(tmp_list,tmp_map);
        destory();
        root=new Node("cipher-1",-1);
        Node* node=new Node("cipher"+to_string(PLSIZE),M);
        root->right=node;
        node->parent=root;
        Update(tmp_list,tmp_map);
        //cout<<"*********execute update:"<<tx.cipher<<endl;
        is_reb=0;
        //Print();
    }
    else if(tx.tx_type==TX)
    {
        int v_aft=version;
        if(v_aft!=tx.v_bef)
        {
            reb_num++;
            //cout<<"abort:"<<tx.cipher<<" version changed,"<<tx.v_bef<<","<<version<<endl;
            return;
        }
        else
        {
            Node* tmp=root;
            string path=tx.path;
            for(int i=0;i<path.length()-1;i++)
            {
                if(tmp!=nullptr)
                {
                    if(path[i]=='0')
                        tmp=tmp->left;
                    else
                        tmp=tmp->right;
                }
                else
                {
                    cerr<<"reb conflict:"<<tx.cipher<<endl;
                    reb_num++;
                    return;
                }
            }
            Node* node=new Node(tx.cipher,tx.y);
            char c=path[path.length()-1];
            if(c=='0')
            {
                if(tmp->left!=nullptr)
                {
//                    Node* e=tmp->left;
//                    if(e->left!=nullptr || e->right!=nullptr)
//                    {
//                        int num=0;
//                        get_childnodes(e,num);
//                        cout<<"!!!!!!!!!!:"<<num<<endl;
//                    }
                    return;
                }
                else
                {
                    node->parent=tmp;
                    tmp->left=node;
                }

            }
            else
            {
                if(tmp->right!=nullptr)
                {
//                    Node* e=tmp->right;
//                    if(e->left!=nullptr || e->right!=nullptr)
//                    {
//                        int num=0;
//                        get_childnodes(e,num);
//                        cout<<"!!!!!!!!!!:"<<num<<endl;
//                    }
                    return;
                }
                else
                {

                    node->parent=tmp;
                    tmp->right=node;
                }
            }
        }
    }
    else
    {
        cerr<<"wrong msg type:"<<tx.tx_type<<endl;
        return;
    }

    tx_block.push(tx);
}
void Inorder4(Node* node,vector<string>& tmp_list,map<string,int64_t>& tmp_map)
{
    if(node==nullptr)
        return;
    Inorder4(node->left,tmp_list,tmp_map);
    if(node->code!=-1 && node->code!=M && node->code!=ERR)
    {
        tmp_list.push_back(node->cipher);
        tmp_map[node->cipher]=node->code;
    }
    Inorder4(node->right,tmp_list,tmp_map);
}
void Inorder4(vector<string>& tmp_list,map<string,int64_t>& tmp_map)
{
    tmp_map[root->cipher]=root->code;
    tmp_map[root->right->cipher]=root->right->code;
    Inorder4(root,tmp_list,tmp_map);
    //    sort(tmp_list.begin(),tmp_list.end(),sortFunLst);
}
