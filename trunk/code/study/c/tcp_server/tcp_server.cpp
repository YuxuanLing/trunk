// tcp_server.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//绑定IP和端口
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(5005);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//开始监听
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}

	//循环接收数据
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[8192];
	while (true)
	{
		printf("waiting connect...\n");
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("rcv one connect: %s \r\n", inet_ntoa(remoteAddr.sin_addr));

		//接收数据
		while (1)
		{
			int ret = recv(sClient, revData, 8192, 0);
			if (ret >= 8192)
			{
				printf("too big \n");
			}
			if (ret > 0)
			{
				revData[ret] = 0x00;
				printf(revData);
			}
			else
			{
				printf("client closed \n");
				break;
			}

			//发送数据
			char * sendData = "Hello, TCP client ! \n";
			send(sClient, sendData, strlen(sendData), 0);
		}
		closesocket(sClient);
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}