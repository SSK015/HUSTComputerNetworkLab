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
#include <regex>


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
        return ".txt";
    }
}

void LoadResources(string path, string& PicName, string&CssName) {
    // std::ifstream inputFile("D:/code/index.html");
    // std::string htmlContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	std::string htmlContent = path;
	std::string htmlContent1 = path;

    std::regex hrefRegex("href\\s*=\\s*\"([^\"]*)\"");
    std::regex srcRegex("src\\s*=\\s*\"([^\"]*)\"");

    std::smatch match;
    std::smatch match1;

    // std::cout << "Matching href attributes:" << std::endl;
    while (std::regex_search(htmlContent, match, hrefRegex)) {
        // std::cout << "href: " << match[1] << std::endl;
        CssName = match[1];
        htmlContent = match.suffix().str();
    }
    // std::ifstream inputFile1("D:/code/index.html");
    // std::string htmlContent1((std::istreambuf_iterator<char>(inputFile1)), std::istreambuf_iterator<char>());

    // htmlContent1 = std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    // std::cout << "\nMatching src attributes:" << std::endl;
    while (std::regex_search(htmlContent1, match1, srcRegex)) {
        // std::cout << "src: " << match1[1] << std::endl;
        PicName = match1[1];
        htmlContent1 = match1.suffix().str();
    }

    // return 0;
}

void loadConfig() {
    ifstream configFile("../client.conf");
    if (configFile.is_open()) {
        getline(configFile, serverAddress);
        configFile >> serverPort;
		// cout << serverPort << endl;
        configFile.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the newline
        getline(configFile, rootDirectory);
		// cout << rootDirectory << endl;
		// getline(configFile, clien);
		configFile >> clientPort;
		// cout << clientPort << endl;
		configFile >> requestedFileName;
		// cout << requestedFileName << endl;
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

	rtn = connect(clientSocket,(LPSOCKADDR)&serverAddr,sizeof(serverAddr));
	if(rtn == SOCKET_ERROR ) {
		printf("Connect to server error!\n");
		return 0;
	}
	// printf("Connect to server ok!");
	int cnt = 0;
	do {
		// loadConfig();
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
			int ResType = 0;
			cin >> requestedFileName;
			const std::string path = requestedFileName;
			// cout << path << endl;
			// cin >> path;
			if (path.find(".html") != std::string::npos) {
				ResType = 1;
			} else {
				// std::cout << "字符串不包含'.html'" << std::endl;
			}
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
			char* fileBuffer = new char[4000001];
			char buffer[409600];
			char PicBuffer[102400];
			char CssBuffer[102400];
			// char request[102400];
			char* request = new char[4000001];
			char PlayBuffer[102400];
			std::string response;
			std::string recvhttp;
			int bytesRead;

			if ((bytesRead = recv(clientSocket, request, 4000000, 0)) < 10000000) {
				recvhttp.append(request, bytesRead);
				// cout << recvhttp << endl;
				// cout << bytesRead << endl;
    			std::string searchString = "404 Not Found";
				std::string searchString1 = "403 Forbidden";
				if (recvhttp.find(searchString) != std::string::npos) {
					std::cout << "The string contains '404 Not Found' " << std::endl;
					// break;
					if ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) < 10000000) {
						response.append(buffer, bytesRead);
						// cout << bytesRead << endl;
					}
					FILE *fp;
					if ((fp = fopen("404.html", "wb")) == NULL) {
						cout << "file not" << endl;
					}
					fwrite(buffer, 1, bytesRead, fp);
					// // 写入文件

					fclose(fp);
					system("404.html");
					continue;
				}

				if (recvhttp.find(searchString1) != std::string::npos) {
					std::cout << "The string contains '403 Not Found'. Breaking loop." << std::endl;
					// break;
					if ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) < 10000000) {
						response.append(buffer, bytesRead);
					}
					FILE *fp;
					if ((fp = fopen("403.html", "wb")) == NULL) {
						cout << "file not" << endl;
					}
					fwrite(buffer, 1, bytesRead, fp);
					fclose(fp);
					system("403.html");
					continue;
				}

			}

			// cout << bytesRead << endl;
			if ((bytesRead = recv(clientSocket, fileBuffer, 4000000, 0)) < 10000000) {	
				response.append(fileBuffer, bytesRead);
				// cout << bytesRead << endl;
			}

			int CssByte, PicByte;
			int tmp;
			if (ResType) {
				if ((tmp = recv(clientSocket, CssBuffer, sizeof(CssBuffer), 0)) < 10000000) {
					// recvhttp.append(request, bytesRead);
					CssByte = tmp;
					// cout << tmp << endl;
				}
				if ((tmp = recv(clientSocket, PicBuffer, sizeof(PicBuffer), 0)) < 10000000) {
					PicByte = tmp;
					// cout << tmp << endl;
					// recvhttp.append(request, bytesRead);
				}
			}

			if (bytesRead == -1) {
				std::cerr << "Error receiving response" << std::endl;
			}

			int fileSize;
			size_t contentLengthPos = recvhttp.find("Content-Length:");

			if (contentLengthPos != std::string::npos) {
				size_t valueStart = contentLengthPos + strlen("Content-Length:");
				size_t valueEnd = recvhttp.find("\r\n", valueStart);

				if (valueEnd != std::string::npos) {
					std::string contentLengthStr = recvhttp.substr(valueStart, valueEnd - valueStart);
						fileSize = std::stoi(contentLengthStr);
				}
			} else {

			}

			cout << "Finish http get context" << endl;

			// cout << fileSize << endl;
			FILE *fp;
			if ((fp = fopen(requestedFileName.c_str(), "wb")) == NULL) {
				// cout << "file not" << endl;
			}
			fwrite(fileBuffer, 1, fileSize, fp);

			fclose(fp);
			std::string PicName, CssName;
			LoadResources(recvhttp, PicName, CssName);
			if (ResType) {
				if ((fp = fopen(PicName.c_str(), "wb")) == NULL) {
					cout << "file not" << endl;
				}

				fwrite(CssBuffer, 1, CssByte, fp);

				fclose(fp);
				if ((fp = fopen(CssName.c_str(), "wb")) == NULL) {
					cout << "file not" << endl;
				}

				fwrite(PicBuffer, 1, PicByte, fp);
				fclose(fp);
			}



			int result = system(requestedFileName.c_str());

			


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
	}while(input != "quit");
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}