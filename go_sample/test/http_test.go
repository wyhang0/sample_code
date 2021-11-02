package test

import (
	"fmt"
	"io"
	"net/http"
	"strconv"
	"testing"
)

func TestHttp(t *testing.T)  {
	// 服务器
	smx := http.NewServeMux()
	smx.HandleFunc("/go", func(writer http.ResponseWriter, request *http.Request) {
		fmt.Println(request.RemoteAddr, "连接成功")
		// 请求方式：GET POST DELETE PUT UPDATE
		fmt.Println("method:", request.Method)
		// /go
		fmt.Println("url:", request.URL.Path)
		fmt.Println("header:", request.Header)
		fmt.Println("body:", request.Body)
		// 回复
		writer.Write([]byte("www.5lmh.com"))
	})
	server := http.Server{Addr: "0.0.0.0:1234", Handler: smx}
	defer server.Close()
	go func() {
		server.ListenAndServe()
	}()

	// 客户端
	resp, _ := http.Get("http://127.0.0.1:1234/go")
	defer resp.Body.Close()
	// 200 OK
	fmt.Println(resp.Status)
	fmt.Println(resp.Header)

	len, _ := strconv.Atoi(resp.Header["Content-Length"][0])
	buf := make([]byte, len)
	for {
		// 接收服务端信息
		n, err := resp.Body.Read(buf)
		if err != nil && err != io.EOF {
			fmt.Println(err)
			return
		} else {
			fmt.Println("读取完毕")
			res := string(buf[:n])
			fmt.Println(res)
			break
		}
	}
}
