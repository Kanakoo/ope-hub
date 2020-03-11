#include "cwrap.h"
#include "tcp-Server.h"
#include "node1.h"
#include "util.h"
#include <iostream>

//void cEncode(int newSd);
//    Encode(newSd);
//}

const char* cjudge(){
    return Judge();
}

void cInit(){
    Init();
}
void cInitSlave(){
    Init_Slave();
}
//void cclearTxBlock(){
//    clearTxBlock();
//}
int cGetBlock()
{
//       Block block;
//	   block.block_id=1;
//	   Tx tx;
//	   tx.tx_type=1;
//	   tx.cipher="test1";
//	   tx.y=65536;
//	   tx.path="10011";
//	   tx.v_bef=1;
//	   block.push(tx);
//	   Tx tx1;
//	   tx1.tx_type=2;
//	   tx1.cipher="test2";
//	   tx1.y=1024;
//	   tx1.path="10110";
//	   tx1.v_bef=2;
//	   block.push(tx1);
	   return getProcessedBlock();
}
void ctest_execute(const char* buf)
{
    Block new_block;
    Block tmp_block = deserialize_block(buf);
    //tmp_block.Print();
    for(int i=0;i<tmp_block.n;i++){
        Tx tx1 = tmp_block.get(i);
        execute_by_type(tx1,new_block);
    }
}
void ctest_execute_slave(const char* buf)
{
    Block tmp_block = deserialize_block(buf);
    execute_by_type_slave(tmp_block);
}