#include <iostream>
// #include <ios>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <winsock2.h>
#include <thread>
#include <limits>
#include <sstream>
#include <vector>
#include <direct.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

string serverAddress = "127.0.0.1";
int serverPort = 67;
string rootDirectory = "D://client0//";

enum {Request, Listen, Surf, Exception};

void loadConfig() {
    ifstream configFile("../server.conf");
    if (configFile.is_open()) {
        getline(configFile, serverAddress);
        configFile >> serverPort;
		cout << serverPort << endl;
        configFile.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the newline
        // getline(configFile, rootDirectory);
        // configFile.close();
    } else {
        cerr << "Error opening config file. Using default values." << endl;
    }
}

int main(){
	WSADATA wsaData;
	string input;

	int nRc = WSAStartup(0x0202,&wsaData);

	if(nRc){
		printf("Winsock  startup failed with error!\n");
	}

	if(wsaData.wVersion != 0x0202){
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");

	loadConfig();
    if (_chdir(rootDirectory.c_str()) != 0) {
        cerr << "Failed to change directory to " << rootDirectory << endl;
        WSACleanup();
        return 1;
    }

	SOCKET clientSocket;
	sockaddr_in serverAddr,clientAddr;

	int addrLen;

	//create socket, Use TCP/UDP protocol
	clientSocket = socket(AF_INET,SOCK_STREAM,0);

	if(clientSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");

	//set client port and ip
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(0);
	clientAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//binding
	int rtn = bind(clientSocket,(LPSOCKADDR)&clientAddr,sizeof(clientAddr));
	if(rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//set server's ip and port
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(serverAddress.c_str());

	cout << "Client local port: " << clientAddr.sin_port << endl;
	rtn = connect(clientSocket,(LPSOCKADDR)&serverAddr,sizeof(serverAddr));
	if(rtn == SOCKET_ERROR )
		printf("Connect to server error!\n");

	printf("Connect to server ok!");
	int cnt = 0;
	do {
		cout << "\nPlease input your message:";
		cin >> input;

		//send data to server
		rtn = send(clientSocket,input.c_str(),input.length(),0);
		if (rtn == SOCKET_ERROR) {
			printf("Send to server failed\n");
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		}
	// if (cnt == 0) {
		const char* requestedFileName = "temp.txt";
		send(clientSocket, requestedFileName, strlen(requestedFileName), 0);

		ofstream outputFile("downloaded_file.txt", ios::binary);
		if (!outputFile.is_open()) {
			cerr << "Error opening local file for writing" << endl;
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		cout << "client come here!" << endl;
		char recvBuf[4096];
		int bytesRead;
		if ((bytesRead = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) > 0) {
			outputFile.write(recvBuf, bytesRead);
			cout << "Client" << endl;
		}

		outputFile.close();
		cout << "after close" << endl;

		if (bytesRead < 0) {
			cerr << "recv() failed with error: " << WSAGetLastError() << endl;
		} else {
			cout << "File downloaded successfully." << endl;
		}
	// }

	// closesocket(clientSocket);

	}while(input != "quit");
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}