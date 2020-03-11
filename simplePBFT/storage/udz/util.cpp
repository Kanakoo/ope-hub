#include "util.h"
#include <unistd.h>
int64_t M=/*281474976710656*/281474976710656;
int64_t ERR=-9999;
string res;
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
//int send_block(int Sd,Block& block)
//{
//    usleep(1);
//    char msg[BLOCK_SIZE];
//    memset(msg, 0, sizeof(msg));
//    block.serialize(msg);
//    int n=send(Sd, (char*)&msg, block.get_serialize_size(), 0);
////    block.Print();
//    //cout<<"block size:"<<block.get_serialize_size()<<","<<n<<endl;
//    return n;
//}
//int rec_block(int Sd,Block& block)
//{
//    char msg[BLOCK_SIZE];
//    memset(msg, 0, sizeof(msg));
//    int n=recv(Sd, (char*)&msg, BLOCK_SIZE, 0);
//    //cout<<"rrrrrrrrcv:"<<n<<endl;
//    block.deserialize(msg);
////    block.Print();
//    return n;
//}
const char* serialize_block(Block& block)
{
    Json::Value blk;
    Json::Value tx_batch;
    for(int i=0;i<block.n;i++)
    {
        Tx tmptx=block.get(i);
        Json::Value tx;
        tx["tx_type"]=tmptx.tx_type;
        if(tmptx.tx_type==TX)
        {
        tx["cipher"]=tmptx.cipher;
        tx["y"]=to_string(tmptx.y);
        }
        tx["path"]=tmptx.path;
        tx["v_bef"]=tmptx.v_bef;
        tx["v_udz"]=tmptx.v_udz;
        tx["mtd"]=tmptx.mtd;
        Json::Value Ludz;
        Json::Value Rudz;
        int lsize=tmptx.lz.list.size();
        int rsize=tmptx.rz.list.size();
        tx["lsize"]=lsize;
        tx["rsize"]=rsize;
        for(int j=0;j<lsize;j++)
        {
            Json::Value Lnode;
            Lnode["cipher"]=tmptx.lz.list.at(j).cipher;
            Lnode["code"]=to_string(tmptx.lz.list.at(j).code);
            Ludz.append(Lnode);
        }
        if(lsize>0)
            tx["lz"]=Ludz;
        for(int j=0;j<rsize;j++)
        {
             Json::Value Rnode;
             Rnode["cipher"]=tmptx.rz.list.at(j).cipher;
             Rnode["code"]=to_string(tmptx.rz.list.at(j).code);
             Rudz.append(Rnode);
        }
        if(rsize>0)
            tx["rz"]=Rudz;
        tx_batch.append(tx);
    }
    blk["tx_batch"]=tx_batch;
    blk["n"]=block.n;
    blk["block_id"]=block.block_id;
    Json::FastWriter writer;
    res = writer.write(blk);
//    if(block.n==1){
//        cout<<res<<endl;
//    }

    return res.c_str();
    //return blk.toStyledString().c_str();
}
//const char* serialize_block(Block& block)
//{
//    Json::Value json_blk;
//    json_blk["block_id"]=block.block_id;
//    json_blk["tx_n"]=block.n;
//    json_blk["end"]=block.end;
//    for(int i=0;i<block.n;i++){
//        json_blk["lns"].append(block.lns[i]);
//        json_blk["rns"].append(block.rns[i]);
//    }
//    for(int i=0;i<block.n;i++)
//    {
//        Tx tx=block.get(i);
//        Json::Value json_tx;
//        json_tx["tx_type"]=tx.tx_type;
//        json_tx["cipher"]=tx.cipher;
//        json_tx["y"]=to_string(tx.y);
//        json_tx["path"]=tx.path;
//        json_tx["v_bef"]=tx.v_bef;
//        json_tx["v_parent"]=tx.v_parent;
//        json_tx["v_udz"]=tx.v_udz;
//        json_tx["mtd"]=tx.mtd;
//        int l_size=block.lns[i];
//        int r_size=block.rns[i];
//        for(int j=0;j<l_size;j++){
//            Json::Value json_node;
//            json_node["cipher"]=tx.lz.list[j].cipher;
//            json_node["code"]=to_string(tx.lz.list[j].code);
//            json_tx["lz"].append(json_node);
//        }
//        for(int j=0;j<r_size;j++){
//            Json::Value json_node;
//            json_node["cipher"]=tx.rz.list[j].cipher;
//            json_node["code"]=to_string(tx.rz.list[j].code);
//            json_tx["rz"].append(json_node);
//        }
//        json_blk["tx_batch"].append(json_tx);
//    }
//    return json_blk.toStyledString().c_str();
//}
Block deserialize_block(const char* buf)
{
    Json::Reader reader;
    Json::Value blk;
    Block block;
    if (!reader.parse(buf, blk)){
            cout << "!!!!!!!!!error parse block!" << endl;
            cout<<buf<<endl;
            return block;
    }
    int n=blk["n"].asInt();
    if(n<=0)
    {
        return block;
    }
    Json::Value json_txbatch=blk["tx_batch"];
    for(int i=0;i<n;i++)
    {
        Json::Value json_tx=json_txbatch[i];
        Tx tx;
        tx.tx_type=json_tx["tx_type"].asInt();
        if(tx.tx_type==TX)
        {
        tx.cipher=json_tx["cipher"].asString();
        string str_y=json_tx["y"].asString();
        tx.y=atoll(str_y.c_str());
        }
        tx.path=json_tx["path"].asString();
        tx.v_bef=json_tx["v_bef"].asInt();
        tx.v_udz=json_tx["v_udz"].asInt();
        tx.mtd=json_tx["mtd"].asBool();
        int lsize=json_tx["lsize"].asInt();
        int rsize=json_tx["rsize"].asInt();
        if(lsize>0)
        {
            Json::Value Ludz=json_tx["lz"];
            for(int j=0;j<lsize;j++)
            {
                 Json::Value Lnode=Ludz[j];
                 string str_code=Lnode["code"].asString();
                 int64_t code=atoll(str_code.c_str());
                 Node tmpnode(Lnode["cipher"].asString(),code);
                 tx.lz.list.push_back(tmpnode);
            }
        }
        if(rsize>0)
        {
            Json::Value Rudz=json_tx["rz"];
            for(int j=0;j<rsize;j++)
            {
                 Json::Value Rnode=Rudz[j];
                 string str_code=Rnode["code"].asString();
                 int64_t code=atoll(str_code.c_str());
                 Node tmpnode(Rnode["cipher"].asString(),code);
                 tx.rz.list.push_back(tmpnode);
            }
        }
        block.push(tx);
    }
    block.block_id=blk["block_id"].asInt();
    return block;
}
//Block deserialize_block(const char* buf)
//{
//    Json::Value json_blk;
//    Json::Reader reader;
//    Block block;
//    if (!reader.parse(buf, json_blk)){
//        cout << "error parse block!" << endl;
//        return block;
//    }
//    else
//    {
//        block.block_id=json_blk["block_id"].asInt();
//        block.n=json_blk["tx_n"].asInt();
//        block.end=json_blk["end"].asInt();
//        for(int i=0;i<block.n;i++)
//        {
//           block.lns.push_back(json_blk["lns"][i].asInt());
//           block.rns.push_back(json_blk["rns"][i].asInt());
//        }
//        for(int i=0;i<block.n;i++)
//        {
//            Tx tx;
//            tx.tx_type=json_blk["tx_batch"][i]["tx_type"].asInt();
//            tx.cipher=json_blk["tx_batch"][i]["cipher"].asString();
//            tx.y=std::atoll(json_blk["tx_batch"][i]["y"].asString().c_str());
//            tx.path=json_blk["tx_batch"][i]["path"].asString();
//            tx.v_bef=json_blk["tx_batch"][i]["v_bef"].asInt();
//            tx.v_parent=json_blk["tx_batch"][i]["v_parent"].asInt();
//            tx.v_udz=json_blk["tx_batch"][i]["v_udz"].asInt();
//            tx.mtd=json_blk["tx_batch"][i]["mtd"].asInt();
//            int l_size=block.lns[i];
//            int r_size=block.rns[i];
//            for(int j=0;j<l_size;j++){
//                Node temp;
//                temp.cipher=json_blk["tx_batch"][i]["lz"][j]["cipher"].asString();
//                temp.code=std::atoll(json_blk["tx_batch"][i]["lz"][j]["code"].asString().c_str());
//                tx.lz.list.push_back(temp);
//            }
//            for(int j=0;j<r_size;j++){
//                Node temp;
//                temp.cipher=json_blk["tx_batch"][i]["rz"][j]["cipher"].asString();
//                temp.code=std::atoll(json_blk["tx_batch"][i]["rz"][j]["code"].asString().c_str());
//                tx.rz.list.push_back(temp);
//            }
//            block.tx_batch.push_back(tx);
//        }
//    }
//    return block;
//}
