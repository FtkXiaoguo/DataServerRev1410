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
	int srcSocket;  // 自分
	int dstSocket;  // 相手

	// sockaddr_in 構造体
	struct sockaddr_in srcAddr;
	struct sockaddr_in dstAddr;
	int dstAddrSize = sizeof(dstAddr);
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

 	while(1){ //ループで回すことによって何度でもクライアントからつなぐことができる

		// 接続の受付け
		printf("接続を待っています\nクライアントプログラムを動かしてください\n");
		dstSocket = accept(srcSocket, (struct sockaddr *) &dstAddr, &dstAddrSize);
		printf("socke:%d, %s から接続を受けました\n", dstSocket,inet_ntoa(dstAddr.sin_addr));

		{
		/*
		* test gethostbyaddr !!!
		*/
			struct sockaddr *from_add = (struct sockaddr *)&dstAddr;
			char client_ip_address[128];
			 sprintf(client_ip_address, "%-d.%-d.%-d.%-d",  // this code is ugly but thread safe
			   ((int) from_add->sa_data[2]) & 0xff,
			   ((int) from_add->sa_data[3]) & 0xff,
			   ((int) from_add->sa_data[4]) & 0xff,
			   ((int) from_add->sa_data[5]) & 0xff);

			char *host_temp = &(from_add->sa_data[2]);

static			char new_str[128];
			strcpy(new_str,host_temp);
			hostent *remote = gethostbyaddr(new_str, 4, 2);
			printf("remote %s \n",remote->h_name);
		}
		while(1){
			//パケットの受信
			numrcv = recv(dstSocket, buffer, sizeof(char)*1024, 0);
			if(numrcv ==0 || numrcv ==-1 ){
				status = closesocket(dstSocket); break;
			}
			buffer[numrcv] = 0;
			printf("recieved: %s\n",buffer);
#if 0
			for (i=0; i< numrcv; i++){ // bufの中の小文字を大文字に変換
				//if(isalpha(buffer[i])) 
					buffer[i] = toupper(buffer[i]);
			}
			// パケットの送信
	 		send(dstSocket, buffer, sizeof(char)*1024, 0);
	 		printf("→ 変換後 %s \n",buffer);
#endif

		}
	}
	// Windows での終了設定
	WSACleanup();

	return(0);
}

