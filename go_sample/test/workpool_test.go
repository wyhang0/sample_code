package test

import (
	"fmt"
	"math/rand"
	"sync"
	"testing"
)
import . "tools"

func TestPool(t *testing.T)  {
	pool := NewWorkPool(8)

	pool.Start()
	if !pool.IsRunning {
		t.Error("pool start error")
	}

	var wg sync.WaitGroup
	wg.Add(2)
	go func() {
		defer wg.Done()
		for i := 1; i < 500; i++ {
			work := NewWork(i,
				func(params ...interface{}) []interface{} {
					tmp, sum := params[0].(int), params[0].(int)
					sum = 0
					for tmp > 0 {
						sum += tmp % 10
						tmp /= 10
					}
					return []interface{}{sum}
				},
				rand.Int())
			// 添加任务
			pool.AddWork(work)
			go func() {
				// 等待结果
				for future := range work.Future{
					if !future{
						t.Error(fmt.Sprintf("work:%v random:%v err:%v\n", work.Id, work.Params[0], work.Err))
					}
				}
			}()
		}
	}()
	go func() {
		defer wg.Done()
		for i := 1; i < 500; i++ {
			work := NewWork(i,
				func(params ...interface{}) []interface{} {
					tmp, sum := params[0].(int), params[0].(int)
					sum = 0
					for tmp > 0 {
						sum += tmp % 10
						tmp /= 10
					}
					panic("division by zero")
					return []interface{}{sum}
				},
				rand.Int())
			// 添加任务
			pool.AddWork(work)
			go func() {
				// 等待结果
				for future := range work.Future{
					if future{
						t.Error(fmt.Sprintf("work:%v random:%v result:%v\n", work.Id, work.Params[0], work.Results[0]))
					}
				}
			}()
		}
	}()
	wg.Wait()

	pool.Stop()
	if pool.IsRunning{
		t.Error("pool stop error")
	}
}

func dopool(n int){
	pool := NewWorkPool(8)

	pool.Start()

	var wg sync.WaitGroup

	for i := 1; i < n; i++ {
		wg.Add(1)
		work := NewWork(i,
			func(params ...interface{}) []interface{} {
				tmp, sum := params[0].(int), params[0].(int)
				sum = 0
				for tmp > 0 {
					sum += tmp % 10
					tmp /= 10
				}
				return []interface{}{sum}
			},
			rand.Int())
		// 添加任务
		pool.AddWork(work)
		go func() {
			// 等待结果
			for range work.Future{
				wg.Done()
			}
		}()
	}

	wg.Wait()

	pool.Stop()
}
func BenchmarkPool100(b *testing.B)  {
	for i := b.N; i > 0; i-- {
		dopool(100)
	}
}
func BenchmarkPool1000(b *testing.B)  {
	for i := b.N; i > 0; i-- {
		dopool(1000)
	}
}
func BenchmarkPool10000(b *testing.B)  {
	for i := b.N; i > 0; i-- {
		dopool(10000)
	}
}


