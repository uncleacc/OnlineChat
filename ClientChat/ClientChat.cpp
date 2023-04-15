#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 1024

char szMsg[BUF_SIZE];

using namespace std;

unsigned SendMsg(void *arg) {
	SOCKET sock = *((SOCKET*)arg);
	while (1) {
		cin >> szMsg;
		if (strcmp(szMsg, "Quit") == 0 || !strcmp(szMsg, "quit")) {
			closesocket(sock);
			exit(0);
		}
		send(sock, szMsg, strlen(szMsg), 0);
	}
	return 0;
}

unsigned RecvMsg(void *arg) {
	SOCKET sock = *((SOCKET*)arg); 
	char msg[BUF_SIZE];
	while (1) {
		int len = recv(sock, msg, sizeof(msg) - 1, 0);
		if (len == -1) {
			return -1;
		}
		msg[len] = 0;
		cout << msg << endl;
	}
	return 0;
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {                            
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return -1;
	}

	SOCKET hSock;
	hSock = socket(AF_INET, SOCK_STREAM, 0);

	//绑定端口
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof servAdr);
	servAdr.sin_family = AF_INET;
	servAdr.sin_port = htons(9999);
	inet_pton(AF_INET, "192.168.245.129", &servAdr.sin_addr);
	//连接服务器
	if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		cout << "connect error" << endl;
		return -1;
	}
	else cout << "请输入用户名：";
	
	//循环发消息
	HANDLE hSendHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&SendMsg, (void*)&hSock, 0, NULL);

	//循环收消息
	HANDLE hRecvHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&RecvMsg, (void*)&hSock, 0, NULL);

	//等待线程结束
	WaitForSingleObject(hSendHand, INFINITE);
	WaitForSingleObject(hRecvHand, INFINITE);

	closesocket(hSock);
	WSACleanup();
	return 0;
}