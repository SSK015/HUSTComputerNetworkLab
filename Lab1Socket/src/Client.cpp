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
string rootDirectory = "D://";

string requestedFileName = "temp.txt";
int option;

enum {Request, Listen, Surf, Exception};

void loadConfig() {
    ifstream configFile("../client.conf");
    if (configFile.is_open()) {
        getline(configFile, serverAddress);
        configFile >> serverPort;
		// cout << serverPort << endl;
        configFile.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the newline
        getline(configFile, rootDirectory);
		cout << rootDirectory << endl;
        configFile.close();
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
	clientAddr.sin_port = htons(5050);
	clientAddr.sin_addr.S_un.S_addr = inet_addr(serverAddress.c_str());
	//binding
	int rtn = bind(clientSocket,(LPSOCKADDR)&clientAddr,sizeof(clientAddr));
	if(rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//set server's ip and port
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(serverAddress.c_str());

	// cout << "Client local port: " << clientAddr.sin_port << endl;
	rtn = connect(clientSocket,(LPSOCKADDR)&serverAddr,sizeof(serverAddr));
	if(rtn == SOCKET_ERROR )
		printf("Connect to server error!\n");

	printf("Connect to server ok!");
	int cnt = 0;
	do {
		char MsgOption;
		cout << "c to continue , q to quit" << endl;
		cin >> MsgOption;
		if (MsgOption == 'q') {
			cout << "quit" << endl;
			break;
		} else if (MsgOption != 'c') {
			cout << "Invalid Instruction!" << endl;
			continue;
		}
		cout << "m for tranfer message," << endl << "s for get page" << endl << "and f for getting file" << endl;
		cin >> MsgOption;
		char MSGString[1];
		MSGString[0] = MsgOption;
		rtn = send(clientSocket,MSGString,sizeof(MSGString),0);
		if (MsgOption == 'm') {
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
		} else if (MsgOption == 's') {
			// 发送URL给服务器
			// const char* url = "https://pfzuo.github.io/homepage/";

			const char* url = "http://127.0.0.1/temp.txt";
			int bytesSent = send(clientSocket, url, strlen(url), 0);
			if (bytesSent < 0) {
				perror("Error sending URL to server");
				closesocket(clientSocket);
				return 1;
			}
			
			char buffer[4096];
			int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
			cout << buffer << endl;
			// 	httpResponse.append(buffer, bytesRead);
			// }
			// cout << bytesRead << endl;

			// 关闭客户端套接字
			// close(clientSocket);

			// size_t bodyStart = httpResponse.find("\r\n\r\n");
			// if (bodyStart != std::string::npos) {
			// 	// 找到HTTP头部和正文之间的空行
			// 	// std::string htmlContent = httpResponse.substr(bodyStart + 4); // +4 to skip the CRLF CRLF
			// 	std::cout << "Received HTML content:" << std::endl;
				// std::cout << httpResponse << std::endl;
			// } else {
			// 	// 没有找到空行，说明HTTP响应格式不正确
			// 	std::cerr << "Invalid HTTP response format - no empty line found." << std::endl;
			// }
			// cout << buffer << endl;

		} else if (MsgOption == 'f') {
			send(clientSocket, requestedFileName.c_str(), strlen(requestedFileName.c_str()), 0);

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
			// cout << "after close" << endl;

			if (bytesRead < 0) {
				cerr << "recv() failed with error: " << WSAGetLastError() << endl;
			} else {
				cout << "File downloaded successfully." << endl;
			}
		} else if (MsgOption == 'r'){
			;
		} else {
			cout << "Wrong message!" << endl;
			continue;
		}



	// if (cnt == 0) {
		// const char* requestedFileName = "temp.txt";

	// }

	// closesocket(clientSocket);

	}while(input != "quit");
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}