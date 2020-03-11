package network

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/simplePBFT/consensus"
	pb "github.com/simplePBFT/protos"
	"github.com/simplePBFT/storage/udz"
	gp "google/protobuf"
	"strings"
	"sync/atomic"
	"time"
)

func (node Node) listenTXs(){
	index := 0
	for {
		time.Sleep(time.Millisecond*30)
		judge := udz.GetBlock()
		if judge ==1 {
			//time.Sleep(time.Microsecond*100000)
			block := udz.Judge()
			index ++
			//check 70000
			node.Consenter.RecvMsg(createTxMsg(int64(index),[]byte(block)),&pb.PeerID{Name: fmt.Sprintf("vp%d",0)})
			if strings.Compare(block,"z")==0{
				fmt.Println("find error")
				fmt.Println(block)
			}
			atomic.AddInt32(&consensus.CanGetNextBlock,1)
		}
		for {
			if atomic.LoadInt32(&consensus.CanGetNextBlock)==0{
				break
			}
			time.Sleep(30*time.Millisecond)
		}
	}
}
func (node Node) mockRequest() {
	//for i:=0;i<10;i++{
		time.Sleep(5e9)
		//for i:=0;i<100;i++{
		//	node.Consenter.RecvMsg(createTxMsg(int64(i)), &pb.PeerID{Name: fmt.Sprintf("vp%d", 0)})
		//}

	//}
	//node.Consenter.RecvMsg(createTxMsg(2), &pb.PeerID{Name: fmt.Sprintf("vp%d", 0)})
	//node.Consenter.RecvMsg(createTxMsg(3), &pb.PeerID{Name: fmt.Sprintf("vp%d", 0)})
}

//func createTxMsg(tag int64) (msg *pb.Message) {
//	tx := createTx(tag)
//	txPacked := marshalTx(tx)
//	msg = &pb.Message{
//		Type:    pb.Message_CHAIN_TRANSACTION,
//		Payload: txPacked,
//	}
//	return
//}
//
//func createTx(tag int64) (tx *pb.Transaction) {
//	txTime := &gp.Timestamp{Seconds: tag, Nanos: 0}
//	tx = &pb.Transaction{Type: pb.Transaction_CHAINCODE_DEPLOY,
//		Timestamp: txTime,
//		Payload:   []byte(fmt.Sprint(tag)),
//	}
//	return
//}

func createTxMsg(tag int64,blockData []byte) (msg *pb.Message) {
	tx := createTx(tag,blockData)
	txPacked := marshalTx(tx)
	msg = &pb.Message{
		Type:    pb.Message_CHAIN_TRANSACTION,
		Payload: txPacked,
	}
	return
}

func createTx(tag int64,blockData []byte) (tx *pb.Transaction) {
	txTime := &gp.Timestamp{Seconds: tag, Nanos: 0}
	tx = &pb.Transaction{Type: pb.Transaction_CHAINCODE_DEPLOY,
		Timestamp: txTime,
		Payload:   blockData,
	}
	return
}

func marshalTx(tx *pb.Transaction) (txPacked []byte) {
	txPacked, _ = proto.Marshal(tx)
	return
}
