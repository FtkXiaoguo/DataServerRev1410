// tstTCPIP.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <process.h>    /* _beginthread, _endthread */


int tstTCP_IP();


int _tmain(int argc, _TCHAR* argv[])
{
		
	tstTCP_IP();
	return 0;
}


#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>




class client_sock_data
{
public:
 
	struct sockaddr_in dstAddr;
// ポート番号，ソケット

int dstSocket;  // 相手
///
int instancID;
 
//
 

};
///////


static unsigned int __stdcall 
ReceiveDataThread(LPVOID    pParam)
{
	client_sock_data *sock_s  = (client_sock_data*)(pParam) ;
	int status;
	// 各種パラメータ
	int numrcv;
	char buffer[1024];
	while(1){
			//パケットの受信
		numrcv = recv(sock_s->dstSocket, buffer, sizeof(char)*1024, 0);
		if(numrcv ==0 || numrcv ==-1 ){
			status = closesocket(sock_s->dstSocket); break;
		}
		buffer[numrcv] = 0;
		//
		numrcv = send(sock_s->dstSocket, buffer, numrcv , 0);
		if(numrcv ==0 || numrcv ==-1 ){
			status = closesocket(sock_s->dstSocket); break;
		}
	
	//	printf("[%d] recieved: %s\n",sock_s->instancID  ,buffer);
		printf("[%d] recieved:  \n",sock_s->instancID  ,buffer);


	} 
	printf("[%d] recieved: end ****** \n",sock_s->instancID);

	delete sock_s ;
	return 0;
}

int tstTCP_IP() 
{
	int i;

	// sockaddr_in 構造体
 
	
	struct sockaddr_in srcAddr;

	int srcSocket;  // 自分

	int dstAddrSize = sizeof(struct sockaddr_in);
	int status;
	// 各種パラメータ
	int numrcv;
	char buffer[1024];

	// Windows の場合
	WSADATA data;
	WSAStartup(MAKEWORD(2,0), &data);

 
	// sockaddr_in 構造体のセット
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_port = htons(105);
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// ソケットの生成（ストリーム型）
	srcSocket = socket(AF_INET, SOCK_STREAM, 0);
 

  	// ソケットのバインド
	bind(srcSocket, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
  	// 接続の許可
	listen(srcSocket, 1);
 

	int instanceID = 0;
	int param_temp = 0; 
	
 
	/*
	* 自分のSocket（Listener)は１つ
	*
	*/

 while(1){ //ループで回すことによって何度でもクライアントからつなぐことができる

	client_sock_data *sock_s = new client_sock_data;
	sock_s->instancID = instanceID++;

 
	// 接続の受付け
	printf("接続を待っています\nクライアントプログラムを動かしてください\n");
	sock_s->dstSocket = accept(srcSocket, (struct sockaddr *) &(sock_s->dstAddr), &dstAddrSize);
	printf("socke:%d, %s から接続を受けました\n", sock_s->dstSocket,inet_ntoa(sock_s->dstAddr.sin_addr));

 
	unsigned int threadId;
	/*  受信スレッド生成            */
     HANDLE hThread = (HANDLE)_beginthreadex(NULL,0,ReceiveDataThread,sock_s,0,&threadId);

	 
 //   WaitForSingleObject(hThread,INFINITE);  /*  受信スレッド終了待ち        */
  //  CloseHandle(hThread);                   /*  ハンドルクローズ            */

		
	}
	// Windows での終了設定
	WSACleanup();


	return(0);
}

