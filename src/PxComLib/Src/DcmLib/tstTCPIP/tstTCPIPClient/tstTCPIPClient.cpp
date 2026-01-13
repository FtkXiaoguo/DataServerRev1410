// tstTCPIP.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
int tstTCP_IP();


int _tmain(int argc, _TCHAR* argv[])
{
		
	tstTCP_IP();
	return 0;
}


#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int tstTCP_IP() 
{
	int i;
	// ポート番号，ソケット
 
	int serverSocket;  

	// sockaddr_in 構造体
//	struct sockaddr_in srcAddr;
	struct sockaddr_in server;
	int dstAddrSize = sizeof(server);
	int status;
	// 各種パラメータ
	int numrcv;
	char buffer[1024];

	// Windows の場合
	WSADATA data;
	WSAStartup(MAKEWORD(2,0), &data);
	// sockaddr_in 構造体のセット
	memset(&server, 0, sizeof(server));
	server.sin_port = htons(105);
	server.sin_family = AF_INET;
//	server.sin_addr.s_addr = htonl(INADDR_ANY);

	unsigned long addr = 0;
	addr = inet_addr("172.17.3.72");
	memcpy(&server.sin_addr, &addr, (size_t) sizeof(addr));


	// ソケットの生成（ストリーム型）
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	 

	int rc = connect(serverSocket, (struct sockaddr *) & server, sizeof(server));

	int nbyte = 10;
	
	char *snd_data = "abcd";
  	 
	 rc = send(serverSocket, snd_data, strlen(snd_data), 0);
	 //
	  snd_data = "1234";
  	 
	 rc = send(serverSocket, snd_data, strlen(snd_data), 0);


	// Windows での終了設定
	WSACleanup();

	return(0);
}

