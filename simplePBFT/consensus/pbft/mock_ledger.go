package pbft

import (
	"fmt"
	"github.com/simplePBFT/consensus"
	"github.com/simplePBFT/storage/udz"
	"reflect"
	"sync"
	"sync/atomic"
	"time"

	"github.com/golang/protobuf/proto"

	"github.com/simplePBFT/protos"
)

type LedgerDirectory interface {
	GetLedgerByPeerID(peerID *protos.PeerID) (consensus.ReadOnlyLedger, bool)
}

type MockLedger struct {
	cleanML       *MockLedger
	blocks        map[uint64]*protos.Block
	blockHeight   uint64
	remoteLedgers LedgerDirectory

	mutex *sync.Mutex

	txID          interface{}
	curBatch      []*protos.Transaction
	curResults    []byte
	preBatchState uint64

	ce *consumerEndpoint // To support the ExecTx stuff
}

func NewMockLedger(remoteLedgers LedgerDirectory) *MockLedger {
	mock := &MockLedger{}
	mock.mutex = &sync.Mutex{}
	mock.blocks = make(map[uint64]*protos.Block)
	mock.blockHeight = 1
	mock.blocks[0] = &protos.Block{}
	mock.remoteLedgers = remoteLedgers

	return mock
}

func (mock *MockLedger) BeginTxBatch(id interface{}) error {
	if mock.txID != nil {
		return fmt.Errorf("Tx batch is already active")
	}
	mock.txID = id
	mock.curBatch = nil
	mock.curResults = nil
	return nil
}

func (mock *MockLedger) Execute(tag interface{}, txs []*protos.Transaction) {
	go func() {
		if mock.txID == nil {
			mock.BeginTxBatch(mock)
		}

		_, err := mock.ExecTxs(mock, txs)
		if err != nil {
			panic(err)
		}
		mock.ce.consumer.Executed(tag)
	}()
}

func (mock *MockLedger) Commit(tag interface{}, meta []byte) {
	go func() {
		_, err := mock.CommitTxBatch(mock, meta)
		if err != nil {
			panic(err)
		}
		mock.ce.consumer.Committed(tag, mock.GetBlockchainInfo())
	}()
}

func (mock *MockLedger) Rollback(tag interface{}) {
	go func() {
		mock.RollbackTxBatch(mock)
		mock.ce.consumer.RolledBack(tag)
	}()
}

func (mock *MockLedger) ExecTxs(id interface{}, txs []*protos.Transaction) ([]byte, error) {
	if !reflect.DeepEqual(mock.txID, id) {
		return nil, fmt.Errorf("Invalid batch ID")
	}

	mock.curBatch = append(mock.curBatch, txs...)
	var err error
	var txResult []byte
	if nil != mock.ce && nil != mock.ce.execTxResult {
		txResult, err = mock.ce.execTxResult(txs)
	} else {
		// This is basically a default fake default transaction execution
		if nil == txs {
			txs = []*protos.Transaction{{Payload: []byte("DUMMY")}}
		}

		for _, transaction := range txs {
			if transaction.Payload == nil {
				transaction.Payload = []byte("DUMMY")
			}

			txResult = append(txResult, transaction.Payload...)
		}

	}

	mock.curResults = append(mock.curResults, txResult...)

	return txResult, err
}

func (mock *MockLedger) CommitTxBatch(id interface{}, metadata []byte) (*protos.Block, error) {
	block, err := mock.commonCommitTx(id, metadata, false)
	if nil == err {
		mock.txID = nil
		mock.curBatch = nil
		mock.curResults = nil
	}
	return block, err
}

func (mock *MockLedger) commonCommitTx(id interface{}, metadata []byte, preview bool) (*protos.Block, error) {
	if !reflect.DeepEqual(mock.txID, id) {
		return nil, fmt.Errorf("Invalid batch ID")
	}

	previousBlockHash := []byte("Genesis")
	if 0 < mock.blockHeight {
		previousBlock, _ := mock.GetBlock(mock.blockHeight - 1)
		previousBlockHash, _ = mock.HashBlock(previousBlock)
	}

	block := &protos.Block{
		ConsensusMetadata: metadata,
		PreviousBlockHash: previousBlockHash,
		StateHash:         mock.curResults, // Use the current result output in the hash
		Transactions:      mock.curBatch,
		NonHashData:       &protos.NonHashData{},
	}

	if !preview {
		//hash, _ := mock.HashBlock(block)
		//fmt.Printf("TEST LEDGER: Mock ledger is inserting block %d with hash %x\n", mock.blockHeight, hash)
		mock.blocks[mock.blockHeight] = block
		//fmt.Println(block.Transactions)
		mock.blockHeight++
		if consensus.TheNodeId != "cmt1_0"{
			//if mock.blockHeight == 2{
			//	go mock.LazyCommit()
			//}
			udz.SendBlock_Slave(string(block.Transactions[0].Payload))
		}else{
			atomic.SwapInt32(&consensus.CanGetNextBlock,0)
		}
	}

	return block, nil
}

func (mock *MockLedger)LazyCommit(){
	time.Sleep(20*time.Second)
	fmt.Println("enter lazy")
	var i uint64
	for i=1;i<mock.blockHeight;i++{
		udz.SendBlock_Slave(string(mock.blocks[i].Transactions[0].Payload))
		if len(string(mock.blocks[i].Transactions[0].Payload)) < 30 {
			fmt.Println("error block may be")
			fmt.Println(string(mock.blocks[i].Transactions[0].Payload))
		}
		fmt.Println(mock.blockHeight)
	}

}
func (mock *MockLedger) PreviewCommitTxBatch(id interface{}, metadata []byte) ([]byte, error) {
	b, err := mock.commonCommitTx(id, metadata, true)
	if err != nil {
		return nil, err
	}
	return mock.getBlockInfoBlob(mock.blockHeight+1, b), nil
}

func (mock *MockLedger) RollbackTxBatch(id interface{}) error {
	if !reflect.DeepEqual(mock.txID, id) {
		return fmt.Errorf("Invalid batch ID")
	}
	mock.curBatch = nil
	mock.curResults = nil
	mock.txID = nil
	return nil
}

func (mock *MockLedger) GetBlockchainSize() uint64 {
	mock.mutex.Lock()
	defer func() {
		mock.mutex.Unlock()
	}()
	return mock.blockHeight
}

func (mock *MockLedger) GetBlock(id uint64) (*protos.Block, error) {
	mock.mutex.Lock()
	defer func() {
		mock.mutex.Unlock()
	}()
	block, ok := mock.blocks[id]
	if !ok {
		return nil, fmt.Errorf("Block not found")
	}
	return block, nil
}

func (mock *MockLedger) HashBlock(block *protos.Block) ([]byte, error) {
	return block.GetHash()
}

func (mock *MockLedger) GetBlockchainInfo() *protos.BlockchainInfo {
	b, _ := mock.GetBlock(mock.blockHeight - 1)
	return mock.getBlockInfo(mock.blockHeight, b)
}

func (mock *MockLedger) GetBlockchainInfoBlob() []byte {
	b, _ := mock.GetBlock(mock.blockHeight - 1)
	return mock.getBlockInfoBlob(mock.blockHeight, b)
}

func (mock *MockLedger) getBlockInfoBlob(height uint64, block *protos.Block) []byte {
	h, _ := proto.Marshal(mock.getBlockInfo(height, block))
	return h
}

func (mock *MockLedger) getBlockInfo(height uint64, block *protos.Block) *protos.BlockchainInfo {
	info := &protos.BlockchainInfo{Height: height}
	info.CurrentBlockHash, _ = mock.HashBlock(block)
	return info
}

func (mock *MockLedger) GetBlockHeadMetadata() ([]byte, error) {
	b, ok := mock.blocks[mock.blockHeight-1]
	if !ok {
		return nil, fmt.Errorf("could not retrieve block from mock ledger")
	}
	return b.ConsensusMetadata, nil
}

func (mock *MockLedger) simulateStateTransfer(info *protos.BlockchainInfo, peers []*protos.PeerID) {
	var remoteLedger consensus.ReadOnlyLedger
	if len(peers) > 0 {
		var ok bool
		remoteLedger, ok = mock.remoteLedgers.GetLedgerByPeerID(peers[0])
		if !ok {
			panic("Asked for results from a peer which does not exist")
		}
	} else {
		panic("TODO, support state transfer from nil peers")
	}
	fmt.Printf("TEST LEDGER skipping to %+v", info)
	p := 0
	if mock.blockHeight >= info.Height {
		panic(fmt.Sprintf("Asked to skip to a block (%d) which is lower than our current height of %d", info.Height, mock.blockHeight))
	}
	for n := mock.blockHeight; n < info.Height; n++ {
		block, err := remoteLedger.GetBlock(n)

		if nil != err {
			n--
			fmt.Printf("TEST LEDGER: Block not ready yet")
			time.Sleep(100 * time.Millisecond)
			p++
			if p > 10 {
				panic("Tried to get a block 10 times, no luck")
			}
			continue
		}

		mock.blocks[n] = block
	}
	mock.blockHeight = info.Height
}
