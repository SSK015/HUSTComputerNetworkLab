#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main(){
	WSADATA wsaData;
	fd_set rfds;				
	fd_set wfds;				
	bool first_connetion = true;

	int nRc = WSAStartup(0x0202,&wsaData);

	if(nRc){
		printf("Winsock  startup failed with error!\n");
	}

	if(wsaData.wVersion != 0x0202){
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");


	SOCKET srvSocket;
	sockaddr_in addr,clientAddr;
	SOCKET sessionSocket;
	int addrLen;
	//create socket
	srvSocket = socket(AF_INET,SOCK_STREAM,0);
	if(srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");
	//set port and ip
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5050);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//binding
	int rtn = bind(srvSocket,(LPSOCKADDR)&addr,sizeof(addr));
	if(rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");
	//listen
	rtn = listen(srvSocket,5);
	if(rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");

	clientAddr.sin_family =AF_INET;
	addrLen = sizeof(clientAddr);
	char recvBuf[4096];

	u_long blockMode = 1;

	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //Set socket to non-blocking mode
		cout << "ioctlsocket() failed with error!\n";
		return 0;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	//initialize set to a empty set
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	FD_SET(srvSocket, &rfds);

	while(true){
		// Prepare for multiple-reuse
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		FD_SET(srvSocket, &rfds);

		if (!first_connetion) {
			//Reset File Discriptor
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
		}
		
		// Watch File Discriptors to report their states.
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);

		//Check if server socket is well prepared to receive new client connection
		if (FD_ISSET(srvSocket, &rfds)) {
			// Reduce Discriptors need to be watching
			nTotal--;

			//Create a new socket to enable communication with client
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET)
				printf("Socket listen one client request!\n");

			//Set socket attribute
			if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { 
				cout << "ioctlsocket() failed with error!\n";
				return 0;
			}
			cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);

			first_connetion = false;

		}


		if (nTotal > 0) {
			if (FD_ISSET(sessionSocket, &rfds)) {
				//receiving data from client
				memset(recvBuf, '\0', 4096);
				rtn = recv(sessionSocket, recvBuf, 256, 0);
				if (rtn > 0) {
					printf("Received %d bytes from client: %s\n", rtn, recvBuf);
				}
				else {
					printf("Client leaving ...\n");
					closesocket(sessionSocket); 
				}

			}
		}	
	}
    return 0;
}