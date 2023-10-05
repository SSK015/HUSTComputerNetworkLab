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
int clientPort = 123456789;
// string fileName = "hello.txt";

string requestedFileName = "temp.txt";
int option;

enum {Request, Listen, Surf, Exception};

std::string createHttpRequest(const std::string& host, const std::string& path) {
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n"; 
    return request;
}

std::string getFileExtension(const std::string& contentType) {
    if (contentType.find("text/html") != std::string::npos) {
        return ".html";
    } else if (contentType.find("image/jpeg") != std::string::npos) {
        return ".jpg";
    } else if (contentType.find("image/png") != std::string::npos) {
        return ".png";
    } else {
        return ".dat";
    }
}

void loadConfig() {
    ifstream configFile("../client.conf");
    if (configFile.is_open()) {
        getline(configFile, serverAddress);
        configFile >> serverPort;
		// cout << serverPort << endl;
        configFile.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the newline
        getline(configFile, rootDirectory);
		cout << rootDirectory << endl;
		// getline(configFile, clien);
		configFile >> clientPort;
		cout << clientPort << endl;
		configFile >> requestedFileName;
		cout << requestedFileName << endl;
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
	clientAddr.sin_port = htons(clientPort);
	clientAddr.sin_addr.S_un.S_addr = inet_addr(serverAddress.c_str());
	//binding
	int rtn = bind(clientSocket,(LPSOCKADDR)&clientAddr,sizeof(clientAddr));
	if(rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//set server's ip and port
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(serverAddress.c_str());
    // u_long nonBlockingMode = 1;
    // if (ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode) == SOCKET_ERROR) {
    //     std::cerr << "Failed to set non-blocking mode" << std::endl;
    //     closesocket(clientSocket);
    //     WSACleanup();
    //     return 1;
    // }

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
        // SOCKET connfd = accept(clientSocket, (LPSOCKADDR)&clientAddr, &addrLen);
        // if (connfd == INVALID_SOCKET) {
        //     cerr << "accept() failed with error: " << WSAGetLastError() << endl;
        //     continue;
        // }
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

			const std::string path = requestedFileName;
			std::string httpRequest = createHttpRequest(serverAddress, path);

			if (send(clientSocket, httpRequest.c_str(), httpRequest.size(), 0) == -1) {
				std::cerr << "Error sending request" << std::endl;
				closesocket(clientSocket);
				return 1;
			}
			// int rcvBufSize = 409600;
			// printf("you want to set udp socket recv buff size to %d\n", rcvBufSize);
			// auto optlen = sizeof(rcvBufSize);
			// if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (const char*)rcvBufSize, optlen) < 0)
			// {
			// 	printf("setsockopt error=%d(%s)!!!\n", errno, strerror(errno));
			// 	// goto error;
			// }
			char buffer[409600];
			std::string response;
			int bytesRead;
			if ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
				response.append(buffer, bytesRead);
			}

			cout << bytesRead << endl;

			if (bytesRead == -1) {
				std::cerr << "Error receiving response" << std::endl;
				// closesocket(clientSocket);
				// return 1;
			}

			cout << "Finish http get context" << endl;
			// 解析Content-Type标头
					FILE *fp;
					if ((fp = fopen(requestedFileName.c_str(), "wb")) == NULL) {
						cout << "file not" << endl;
					}
					// int n;
					// while (1) {
					// 	n = recv(clientSocket, buffer, sizeof(buffer), 0);
					// 	if (n <= 0) {
					// 		break;
					// 	}

					// }
					// cout << buffer << endl;
					fwrite(buffer, 1, sizeof(buffer), fp);
					// // 写入文件
					fclose(fp);
			// size_t contentTypeStart = response.find("Content-Type:");
			// if (contentTypeStart != std::string::npos) {
			// 	size_t contentTypeEnd = response.find("\r\n", contentTypeStart);
			// 	if (contentTypeEnd != std::string::npos) {
			// 		std::string contentType = response.substr(contentTypeStart + 13, contentTypeEnd - (contentTypeStart + 13));
			// 		std::string fileExtension = getFileExtension(contentType);
			// 		FILE *fp;
			// 		if ((fp = fopen("D:/client0/new1.jpg", "ab")) == NULL) {
			// 			cout << "file not" << endl;
			// 		}
			// 		// int n;
			// 		// while (1) {
			// 		// 	n = recv(clientSocket, buffer, sizeof(buffer), 0);
			// 		// 	if (n <= 0) {
			// 		// 		break;
			// 		// 	}

			// 		// }
			// 		// cout << buffer << endl;
			// 		fwrite(buffer, 1, sizeof(buffer), fp);
			// 		// // 写入文件
			// 		fclose(fp);
			// 		// std::ofstream outputFile("received_file" + fileExtension, std::ios::binary);
			// 		// if (!outputFile.is_open()) {
			// 		// 	std::cerr << "Error opening output file" << std::endl;
			// 		// 	closesocket(clientSocket);
			// 		// 	return 1;
			// 		// }
			// 		// outputFile.write(response.c_str(), response.size());
			// 		// outputFile.close();
			// 	}
			// }
			


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