// Server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	//初始化
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建套接字
	SOCKET serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 设置地址信息
	sockaddr_in sockAddr;
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockAddr.sin_port = htons(1234);

	// 绑定套接字
	bind(serverSock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));

	// 开始监听
	listen(serverSock, 20);

	SOCKADDR clientAddr;
	int nSize = sizeof(SOCKADDR);
	while(1)
	{
		// 接收客户端请求，生成套接字
		SOCKET clientSock = accept(serverSock, &clientAddr, &nSize);

		char str[] = "hello Client, this message come from server";

		// 向客户端发送数据
		send(clientSock, str, strlen(str) + 1, 0);

		// 接收客户端的数据
		char recvBuffer[1024] = { 0 };
		recv(clientSock, recvBuffer, 1024, 0);

		std::cout << "recive message: " << recvBuffer << std::endl;

		// 关闭客户端这一次的套接字
		closesocket(clientSock);


	}
	closesocket(serverSock);

	WSACleanup();

    std::cout << "Hello World!\n"; 
}

/*
 * 服务端流程：
 * 1. 初始化
 * 2. 创建套接字
 * 3. 设置地址信息等
 * 4. bind 绑定
 * 5. listen 监听
 * 6. accept 接收客户端请求
 * 7. recive 接收客户端数据
 * 8. send 向客户端发送数据
 * 9. 关闭此次客户端请求套接字
 * 10. 关闭服务端套接字
 * 11. 清理
 */
