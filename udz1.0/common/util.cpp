#include "util.h"
#include <unistd.h>
int64_t M=281474976710656/*2251799813685248*/;
int64_t ERR=-9999;
//int send_msg(int Sd,send_item& item)
//{
//    usleep(1);
//    char msg[item.get_serialize_size()];
//    memset(msg, 0, sizeof(msg));
//    item.serialize(msg);
//    int n=send(Sd, (char*)&msg, item.get_serialize_size(), 0);
//    //cout<<"send:"<<item.cipher<<","<<item.get_serialize_size()<<","<<n<<endl;
//    return n;
//}
//int rec_msg(int Sd,send_item& item)
//{
//    char msg[ITEM_SIZE];
//    memset(msg, 0, sizeof(msg));
//    int n=recv(Sd, (char*)&msg, ITEM_SIZE, 0);
//    item.deserialize(msg);
//    //cout<<"rec:"<<item.cipher<<","<<item.get_serialize_size()<<", "<<n<<endl;
//    return n;
//}
int send_msg(int Sd,myPkg& pkg)
{
    usleep(1);
    char buff[BUFF_SIZE];
    memset(buff,0,sizeof(buff));
//    cout<<"------send pkg-----"<<endl;
//    cout<<pkg.msgtype()<<endl;
//    cout<<pkg.encid()<<endl;
//    cout<<pkg.msgid()<<endl;
//    cout<<pkg.cipher()<<endl;
//    cout<<pkg.tmp()<<endl;
//    cout<<"------end-------"<<endl;

    pkg.SerializeToArray(buff,BUFF_SIZE);
    int n=send(Sd,buff,sizeof(buff),0);
    if(n < 0)
    {
        cout<<"send failed ..."<<endl;
    }
    return n;
}
int rec_msg(int Sd, myPkg &pkg)
{
    char buff[BUFF_SIZE];
    memset(buff,0,sizeof(buff));
    int n=recv(Sd,buff,sizeof(buff),0);
    if(n < 0)
    {
        cout<<"recv failed ..."<<endl;
    }
    //protobuf反序列化
    pkg.ParseFromArray(buff,BUFF_SIZE);
//    cout<<"------rec pkg-----"<<endl;
//    cout<<pkg.msgtype()<<endl;
//    cout<<pkg.encid()<<endl;
//    cout<<pkg.msgid()<<endl;
//    cout<<pkg.cipher()<<endl;
//    cout<<pkg.tmp()<<endl;
//    cout<<"------end-------"<<endl;
    return n;
}
int send_block(int Sd,Block& block)
{
    usleep(1);
    char msg[B_SIZE];
    memset(msg, 0, sizeof(msg));
    block.serialize(msg);
    int n=send(Sd, (char*)&msg, block.get_serialize_size(), 0);
//    block.Print();
    //cout<<"block size:"<<block.get_serialize_size()<<","<<n<<endl;
    return n;
}
int rec_block(int Sd,Block& block)
{
    char msg[B_SIZE];
    memset(msg, 0, sizeof(msg));
    int n=recv(Sd, (char*)&msg, B_SIZE, 0);
    //cout<<"rrrrrrrrcv:"<<n<<endl;
    block.deserialize(msg);
//    block.Print();
    return n;
}
