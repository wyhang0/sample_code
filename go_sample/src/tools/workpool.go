package tools

import (
	"sync"
)

type WorkFunction func(params ...interface{}) []interface{}

type Work struct {
	Id int
	Function WorkFunction
	Params []interface{}
	Results []interface{}
	Err interface{}
	Future chan bool
}

type WorkPool struct {
	MaxPoolNum int
	WorkChan chan *Work
	IsRunning bool
	wg sync.WaitGroup
	mt sync.Mutex
}

func NewWork(id int, function WorkFunction, params ...interface{}) *Work {
	return &Work{
		id,
		function,
		params,
		nil,
		nil,
		make(chan bool, 1),
	}
}

func NewWorkPool(maxPoolNum int) *WorkPool  {
	workChanCapacity := map[bool]int{
		maxPoolNum<64: 64,
		maxPoolNum>=64: 256,
	}
	return &WorkPool{
		MaxPoolNum: maxPoolNum,
		WorkChan: make(chan *Work, workChanCapacity[true]),
	}
}

func (w *WorkPool) Start(){
	if w.IsRunning {
		return
	}

	w.mt.Lock()
	defer w.mt.Unlock()

	if !w.IsRunning {
		num := w.MaxPoolNum
		for ; num > 0; num-- {
			w.wg.Add(1)
			go func() {
				defer w.wg.Done()
				for work :=range w.WorkChan{
					func(){
						defer func() {
							if work.Err = recover(); work.Err!=nil{
								work.Future <- false
							}else{
								work.Future <- true
							}
						}()
						work.Results = work.Function(work.Params...)
					}()
				}
			}()
		}
		w.IsRunning = true
	}
}
func (w *WorkPool) Stop() {
	if !w.IsRunning{
		return
	}

	w.mt.Lock()
	defer w.mt.Unlock()

	if w.IsRunning {
		close(w.WorkChan)
		w.wg.Wait()
		w.IsRunning = false
	}
}
func (w *WorkPool) AddWork(work *Work) {
	w.WorkChan <- work
}
