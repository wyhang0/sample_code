package test

import (
	"os"
	"os/signal"
	"syscall"
	"testing"
	"time"
)

func TestSignal(t *testing.T) {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT)

	go func() {
		time.Sleep(3 * time.Second)
		sigs <- syscall.SIGINT
	}()

	sig, _ := <- sigs
	if sig != syscall.SIGINT{
		t.Error("error signal")
	}
}
