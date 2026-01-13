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
	char buffer1[256];
	char recv_buffer[1024];

	int run_i = 0;
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

	int nbyte = 800;
	
	for(int i=0;i<nbyte;i++){
		buffer[i] = 'A'+i%10;
	}
	buffer[nbyte] = 0;
	while(true){
		 
		rc = send(serverSocket, buffer, nbyte, 0);
		 //
		numrcv = recv(serverSocket, recv_buffer,  1024, 0);
		if(numrcv ==0 || numrcv ==-1 || numrcv!=nbyte ){
			printf("recv error1 \n");
		}else{
			for(int i=0;i<nbyte;i++){
				if(recv_buffer[i] != buffer[i]){
					printf("recv data1 [%] error \n",i);
					break;
				}
			}
		}

	 
		sprintf(buffer1," - %d",run_i);
	   	int str_len = strlen(buffer1);
		rc = send(serverSocket, buffer1, str_len, 0);
		numrcv = recv(serverSocket, recv_buffer,  1024, 0);
		if(numrcv ==0 || numrcv ==-1 || numrcv!=str_len ){
			printf("recv error2 \n");
		}else{
			for(int i=0;i<str_len;i++){
				if(recv_buffer[i] != buffer1[i]){
					printf("recv data2 [%] error \n",i);
					break;
				}
			}
		}

		 run_i++;

	//	 ::Sleep(500);
	}

	// Windows での終了設定
	WSACleanup();

	return(0);
}

