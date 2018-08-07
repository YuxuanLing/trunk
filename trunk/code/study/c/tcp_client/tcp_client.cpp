// tcp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WINSOCK2.H>
#include <STDIO.H>

#pragma  comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
	int c = 0;
	
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("10.140.112.89");


	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}

	while (c <= 10)
	{
		char * sendData = "Hello, TCP server, i am client !\n";
		int ret = 0;
		ret = send(sclient, sendData, strlen(sendData), 0);

		char recData[255];
		ret = recv(sclient, recData, 255, 0);
		if (ret > 0)
		{
			recData[ret] = 0x00;
			printf(recData);
		}

		c++;

    }

	closesocket(sclient);
	Sleep(3);
	WSACleanup();
	return 0;
}
