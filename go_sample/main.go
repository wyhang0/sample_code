package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"
	"tools"
)

func GetCurrentThreadId() int {
	var user32 *syscall.DLL
	var GetCurrentThreadId *syscall.Proc
	var err error

	user32, err = syscall.LoadDLL("Kernel32.dll")
	if err != nil {
		fmt.Printf("syscall.LoadDLL fail: %v\n", err.Error())
		return 0
	}
	GetCurrentThreadId, err = user32.FindProc("GetCurrentThreadId")
	if err != nil {
		fmt.Printf("user32.FindProc fail: %v\n", err.Error())
		return 0
	}

	var pid uintptr
	pid, _, err = GetCurrentThreadId.Call()

	return int(pid)
}

func main() {
	fmt.Println(tools.Work{})
	n := time.Now()
	go func() {
		var j = 0
		for {
			j++
			time.Sleep(1 * time.Second)
			fmt.Println("b", GetCurrentThreadId(), time.Since(n), j)
		}
	}()
	go func() {
		var j = 0
		for {
			j++
			time.Sleep(1 * time.Second)
			fmt.Println("a", GetCurrentThreadId(), time.Since(n), j)
		}
	}()

	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT)

	go func() {
		time.Sleep(3 * time.Second)
		sigs <- syscall.SIGINT
	}()

	sig, _ := <- sigs
	if sig != syscall.SIGINT{
		fmt.Println("error signal")
	}
}
