#ifndef __CWRAP_H__
#define __CWRAP_H__

#ifdef __cplusplus
extern "C" {
#endif
//int cEncode(int newSd);
void cInit();
void cInitSlave();
int cGetBlock();
//void cclearTxBlock();
void ctest_execute(const char* buf);
void ctest_execute_slave(const char* buf);
const char* cjudge();
#ifdef __cplusplus
}
#endif

#endif