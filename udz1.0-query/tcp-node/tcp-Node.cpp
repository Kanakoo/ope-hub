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
#include <algorithm>
#include <assert.h>
#include "../common/util.h"
using namespace std;
//add lqq
//variables
#define EXE_TX 1
#define NUM_REC 1
#define NUM_QRY 0

//std::atomic<int> version{0};
//std::atomic<int> version_reb{0};
std::atomic<int> abort_num{0};
std::atomic<int> conflict_num{0};
std::atomic<int> reb_num{0};
Node* root=new Node();
//bool is_reb=0;
//std::queue<Tx> tx_buffer;
//Block tx_block;

//for update
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
void Inorder1(Node* node,int& num)
{
    if(node==nullptr)
        return;
    Inorder1(node->left,num);

    if(node->get_lzise()>0)
    {
        num+=node->get_lzise();
        cout<<"------left--------of "<<node->cipher<<endl;
        for(int i=0;i<node->get_lzise();i++)
        {
            Node tmp1=node->lz.list[i];
            cout<<"entry: "<<"x="<<tmp1.cipher<<",y="<<tmp1.code<<endl;

        }
        cout<<"-------end-------"<<endl;
    }
    if(node->get_rzise()>0)
    {
        num+=node->get_rzise();
        cout<<"------right--------of "<<node->cipher<<endl;
        for(int i=0;i<node->get_rzise();i++)
        {
            Node tmp2=node->rz.list[i];
            cout<<"entry: "<<"x="<<tmp2.cipher<<",y="<<tmp2.code<<endl;

        }
        cout<<"-------end-------"<<endl;
    }
    cout<<"entry: "<<"x="<<node->cipher<<",y="<<node->code<<endl;
    num++;
    Inorder1(node->right,num);
}
void Print(int& num)
{
    num=0;
    cout<<"====begin print===="<<endl;
    Inorder1(root,num);
    num-=2;
    cout<<"====end print===="<<endl;
    cout<<"total num: "<<num<<endl;
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


void Update(vector<string>& data,map<string,int64_t>& tmp_map);

void execute_tx_m1_slave(Tx& tx)
{
    cout<<"*********execute by m1:"<<tx.cipher<<","<<tx.tx_type<<endl;
    if(tx.tx_type==REB_TX)
    {
        vector<string> tmp_list;
        map<string,int64_t> tmp_map;
        Inorder4(tmp_list,tmp_map);
        destory();
        root=new Node("cipher-1",-1);
        Node* node=new Node("cipher"+to_string(PLSIZE),M);
        root->right=node;
        node->parent=root;
        Update(tmp_list,tmp_map);
    }
    else if(tx.tx_type==TX)
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
        Node* node=new Node(tx.cipher,tx.y);
        char c=path[path.length()-1];
        if(c=='0')
        {
            if(tmp->left!=nullptr)
            {
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
                return;
            }
            else
            {
                node->parent=tmp;
                tmp->right=node;
            }
        }
    }
    else
    {
        cerr<<"wrong msg type:"<<tx.tx_type<<endl;
        return;
    }
}
void execute_tx_m0_slave(Tx& tx)
{
//    cout<<"*********execute by m0:"<<tx.cipher<<","<<tx.tx_type<<endl;
    if(tx.tx_type==REB_TX)
    {
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

    }
    else if(tx.tx_type==RESORT_TX)
    {
        //cout<<"------begin to sort UDZ "<<tx.cipher<<endl;
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
            if(tmp->get_lzise()>0)
            {
                vector<Node> llist=tx.lz.list;
                tmp->lz.list.clear();
                int lcount=llist.size();
                Node* childtree=CreateBST(0,lcount-1,llist);
                tmp->left=childtree;
                childtree->parent=tmp;
            }
            else
            {
                //cout<<"------lalready sorted"<<endl;
                //                            tmp->lock=0;
                return;
            }
        }
        else
        {
            assert(c=='1');
            if(tmp->get_rzise()>0)
            {
                vector<Node> rlist=tx.rz.list;
                tmp->rz.list.clear();
                int rcount=rlist.size();
                Node* childtree=CreateBST(0,rcount-1,rlist);
                tmp->right=childtree;
                childtree->parent=tmp;
            }
            else
            {
                return;
            }
        }
    }
    else if(tx.tx_type==TX)
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
            else
            {
                return;
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
                {
                    tmp->push_l(*node);
                }

            }
            else if(tmp->get_lzise()>0 && tmp->get_lzise()>=UDZ_SIZE)
            {
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
                    return;
                }
                else
                {
                    tmp->push_l(*e);
                    tmp->left=nullptr;
                    tmp->push_l(*node);
                }
            }
            else if(tmp->get_lzise()<=0 && tmp->left==nullptr)
            {
                node->parent=tmp;
                tmp->left=node;
            }
            else
            {
                return;
            }

        }
        else
        {
            if(tmp->get_rzise()>0 && tmp->get_rzise()<UDZ_SIZE)
            {
                {
                    tmp->push_r(*node);
                }

            }
            else if(tmp->get_rzise()>0 && tmp->get_rzise()>=UDZ_SIZE)
            {
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
                    return;
                }
                else
                {
                    tmp->push_r(*e);
                    tmp->right=nullptr;
                    tmp->push_r(*node);
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
    else
    {
        return;
    }
}
void execute_by_type_slave(Block& tx_block)
{
    //cout<<"--------------------tx method:"<<tx.mtd<<endl;
    for(int i=0;i<tx_block.n;i++)
    {
        Tx tx=tx_block.get(i);
        if(tx.mtd==0)
            execute_tx_m0_slave(tx);
        else
            execute_tx_m1_slave(tx);
    }

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

void* write_tree(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    //cout << "Do Rec Thread ID : " << my_data->thread_id<<endl;
    while(1)
    {
        Block block;
        int n=rec_block(my_data->Sd,block);
        if(n>0)
        {
            execute_by_type_slave(block);
//            block.Print();
//            int num=0;
//            Print(num);
        }

    }

}

int main(int argc, char *argv[])
{
    root=new Node("cipher-1",-1);
    Node* node=new Node("cipher"+to_string(PLSIZE),M);
    root->right=node;
    node->parent=root;
    //grab the port number
    int port = 8081/*atoi(argv[1])*/;
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
        cerr << "Error accepting request from Server!" << endl;
        exit(1);
    }
    cout << "Connected with Server!" << endl;
    //lets keep track of the session time
    pthread_t tid3[EXE_TX];
    struct thread_data td3[EXE_TX];

    for(int i=0; i < EXE_TX; i++ ){
        td3[i].thread_id = i;
        td3[i].Sd = newSd;
        int rc = pthread_create(&tid3[i], nullptr,write_tree, (void *)&td3[i]);
        pthread_detach(tid3[i]);
        if (rc){
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    while(1)
    {
        sleep(15);
        int num=0;
        Print(num);
//        abort_num=N-num;
//        conflict_num=abort_num-reb_num;
//        cout<<"abort num: "<<abort_num<<", conflict_num: "<<conflict_num<<", reb_num: "<<reb_num<<endl;
    }
    //we need to close the socket descriptors after we're all done
    close(newSd);
    close(serverSd);
    //    cout << "********Session********" << endl;
    //    cout << "Elapsed time: " << cost
    //        << " ms" << endl;
    //    cout << "Connection closed..." << endl;
    pthread_exit(nullptr);
    return 0;
}
