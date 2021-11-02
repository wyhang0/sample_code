package test

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"reflect"
	"testing"
	"time"
)

// Encode 将消息编码
func Encode(message string) ([]byte, error) {
	// 读取消息的长度，转换成int32类型（占4个字节）
	var length = int32(len(message))
	var pkg = new(bytes.Buffer)
	// 写入消息头
	err := binary.Write(pkg, binary.BigEndian, length)
	if err != nil {
		return nil, err
	}
	// 写入消息实体
	err = binary.Write(pkg, binary.BigEndian, []byte(message))
	if err != nil {
		return nil, err
	}
	return pkg.Bytes(), nil
}

// Decode 解码消息
func Decode(reader io.Reader) (string, error) {
	lengthByte := make([]byte, 4)
	_, err := io.ReadFull(reader, lengthByte)
	if err != nil {
		return "", err
	}
	lengthBuff := bytes.NewBuffer(lengthByte)
	var length int32
	err = binary.Read(lengthBuff, binary.BigEndian, &length)
	if err != nil {
		return "", err
	}

	// 读取真正的消息数据
	pack := make([]byte, length)
	_, err = io.ReadFull(reader, pack)
	if err != nil {
		return "", err
	}
	return string(pack), nil
}

func handle(conn net.Conn) {
	defer conn.Close()
	for {
		msg, err := Decode(conn)
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Println("decode msg failed, err:", err)
			return
		}
		fmt.Println("收到client发来的数据：", msg)
	}
	fmt.Println("关闭 server conn")
}

func TestTcp(t *testing.T){
	// server
	listen, err := net.ListenTCP("tcp", &net.TCPAddr{IP: net.IPv4zero, Port: 1234})
	if err!=nil{
		fmt.Println("listen failed, err:", err)
		return
	}
	defer listen.Close()
	go func() {
		for true {
			conn, err := listen.Accept()
			if err != nil{
				if !reflect.ValueOf(listen).Elem().FieldByIndex([]int{0,3}).Bool(){
					fmt.Println("tcp listen closed")
					return
				}
				fmt.Println("accept failed, err:", err, reflect.TypeOf(err))
				continue
			}
			go handle(conn)
		}
	}()
	// client
	conn, err := net.Dial("tcp", "127.0.0.1:1234")
	defer conn.Close()
	if err != nil{
		fmt.Println("client connect failed, err:", err)
		return
	}
	data, err := Encode("你好！服务器，我在线了")
	if err != nil{
		fmt.Println("encode failed, err:", err)
		return
	}
	conn.Write(data[:1])
	time.Sleep(1*time.Second)
	conn.Write(data[1:2])
	time.Sleep(1*time.Second)
	conn.Write(data[2:3])
	time.Sleep(1*time.Second)
	conn.Write(data[3:4])
	time.Sleep(1*time.Second)
	conn.Write(data[4:len(data)/2])
	time.Sleep(1*time.Second)
	conn.Write(data[len(data)/2:])
	conn.Close()

	time.Sleep(3*time.Second)
}

func TestUdp(t *testing.T){
	// server
	listen, err := net.ListenUDP("udp", &net.UDPAddr{Port: 1234, IP: net.IPv4(0,0,0,0)})
	if err!=nil{
		fmt.Println("listen failed, err:", err)
		return
	}
	defer listen.Close()
	go func() {
		var data [1024]byte
		for true {
			n, addr, err := listen.ReadFromUDP(data[:]) // 接收数据
			if err != nil {
				if !reflect.ValueOf(listen).Elem().FieldByIndex([]int{0, 0, 3}).Bool(){
					fmt.Println("upd listen closed")
					break
				}
				fmt.Println("read udp failed, err:", err)
				continue
			}
			fmt.Printf("data:%v addr:%v count:%v\n", string(data[:n]), addr, n)
			_, err = listen.WriteToUDP(data[:n], addr) // 发送数据
			if err != nil {
				fmt.Println("write to udp failed, err:", err)
				continue
			}
		}
	}()

	// client
	socket, err := net.DialUDP("udp", nil, &net.UDPAddr{IP: net.IPv4(127,0,0,1), Port: 1234})
	if err != nil {
		fmt.Println("连接服务端失败，err:", err)
		return
	}
	defer socket.Close()
	sendData := []byte("Hello server")
	_, err = socket.Write(sendData) // 发送数据
	if err != nil {
		fmt.Println("发送数据失败，err:", err)
		return
	}
	data := make([]byte, 4096)
	n, remoteAddr, err := socket.ReadFromUDP(data) // 接收数据
	if err != nil {
		fmt.Println("接收数据失败，err:", err)
		return
	}
	fmt.Printf("recv:%v addr:%v count:%v\n", string(data[:n]), remoteAddr, n)

	time.Sleep(3*time.Second)
}