package udz
// #cgo LDFLAGS: -lmuduo_net
// #cgo LDFLAGS: -lmuduo_base
// #cgo LDFLAGS: -lpthread
// #cgo LDFLAGS: -lprotobuf
// #cgo LDFLAGS: -ljsoncpp
// #cgo CXXFLAGS: -std=c++11
// #cgo CXXFLAGS: -std=c++1y
// #include "cwrap.h"
import "C"

func Judge() string{
	var res = C.cjudge()
	str := C.GoString(res)
	return str;
}
func GetBlock() int{
	return int(C.cGetBlock())
}

func SendBlock(str string){
	C.ctest_execute(C.CString(str));
}
func Init(){
	C.cInit()
}
func SendBlock_Slave(str string){
	C.ctest_execute_slave(C.CString(str));
}
func Init_Slave(){
	C.cInitSlave()
}