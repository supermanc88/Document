// Client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{

	WSADATA	wsaData;
	// 初始化
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建socket套接字	socket流 TCP协议
	SOCKET	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 设置请求目标
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = PF_INET;	//使用IPv4
	sockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");	//将字符串地址转换成数值
	sockAddr.sin_port = htons(1234);

	// 开始连接
	connect(sock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));

	// 接收服务器传回的数据
	char szBuffer[1024] = { 0 };
	recv(sock, szBuffer, 1023, NULL);

	// 打印接收到的数据
	std::cout << "message form server:" << szBuffer << std::endl;

	// 发送消息
	char str[] = "this message come from Client";
	send(sock, str, strlen(str) + 1, 0);

	// 关闭套接字
	closesocket(sock);

	// 清理
	WSACleanup();

    std::cout << "Hello World!\n"; 
}

/*
 * 客户端流程：
 * 1. 初始化
 * 2. 创建套接字
 * 3. 设置地址信息
 * 4. connet 连接服务端
 * 5. recive 接收服务端发来的数据
 * 6. send 向服务端发送数据
 * 7. 关闭套接字
 * 8. 清理
 */