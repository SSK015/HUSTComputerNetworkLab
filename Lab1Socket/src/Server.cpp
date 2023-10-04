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

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int MAX_CLIENTS = 5;
enum {Request, Listen, Surf, Exception, ReloadConfig};
string serverAddress = "127.0.0.1";
int serverPort = 67;
string rootDirectory = "/";
int FLAG =0;
// string root

void forwardRequest(SOCKET clientSocket, const std::string& targetURL) {
    size_t hostStart = targetURL.find("://");
    if (hostStart == std::string::npos) {
        std::cerr << "Invalid URL format" << std::endl;
        return;
    }
    hostStart += 3; // skip "://" 
    size_t hostEnd = targetURL.find('/', hostStart);
    std::string host = targetURL.substr(hostStart, hostEnd - hostStart);
    cout << host << " sxs" << endl;

    // set connection to the target
    struct hostent* targetHost = gethostbyname(host.c_str());
    if (targetHost == nullptr) {
        perror("Error resolving host");
        return;
    }

    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (targetSocket < 0) {
        perror("Error creating socket to target");
        return;
    }
    // int nRecvBuf=65535;//设置为32K
    // setsockopt(targetSocket,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));



    struct sockaddr_in targetAddr;
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(80);
    targetAddr.sin_addr = *((struct in_addr*)targetHost->h_addr);

    if (connect(targetSocket, (struct sockaddr*)&targetAddr, sizeof(targetAddr)) < 0) {
        perror("Error connecting to target");
        closesocket(targetSocket);
        return;
    }

    // std::string request = "GET " + targetURL.substr(hostEnd) + "index.html" + " HTTP/1.1\r\n";
    // // cout << request << endl;
    // request += "Host: " + host + "\r\n";
    // // request += "User-Agent: MyHttpClient/1.0\r\n";
    // request += "Connection: keep-alive\r\n\r\n";
    // cout << request << endl;

    int bytesSent = send(targetSocket, request.c_str(), request.length(), 0);
    if (bytesSent < 0) {
        perror("Error sending request to target");
        closesocket(targetSocket);
        return;
    }

    // char buffer[40960];
    // std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nHello, World!";
    // send(clientSocket, response.c_str(), response.length(), 0);

    // std::string httpResponse;
    // while (true) {
    //     memset(buffer, 0, sizeof(buffer));
    //     int bytesRead = recv(targetSocket, buffer, 4096, 0);
    //     // id (byte)
    //     if (bytesRead <= 0) {
            
    //         break;
    //     }
    //     httpResponse.append(buffer, bytesRead);
    //     cout << bytesRead << endl;
    //     cout << buffer << endl;
    //     // cout << 1595 << endl;
    // }
    // cout << "exit " << endl;
    // cout << httpResponse << endl; 

    // while (true) {
    //     bytesSent = send(clientSocket, buffer, bytesRead, 0);
    //     if (bytesSent < 0) {
    //         perror("Error sending response to client");
    //         // break;
    //     }
    // }

    closesocket(targetSocket);
}


void loadConfig() {
    ifstream configFile("../server.conf");
    if (configFile.is_open()) {
        getline(configFile, serverAddress);
        configFile >> serverPort;
		cout << serverPort << endl;
        configFile.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the newline
        getline(configFile, rootDirectory);
        configFile.close();
    } else {
        cerr << "Error opening config file. Using default values." << endl;
    }
}

void handleClient(SOCKET clientSocket, sockaddr_in clientAddr) {
    char recvBuf[4096];
    char MSG[1];

    while (true) {
        int rtn = recv(clientSocket, MSG, sizeof(MSG), 0);
        cout << MSG << endl;
        cout << rtn << endl;
        if (rtn > 0) {
			// unsigned short clientPort = ntohs(clientAddr.sin_port);
            // cout << "Received " << rtn << " bytes from client: " << "Port: " << clientPort << " msg: " << recvBuf << endl;
        } else if (rtn == 0) {
            cout << "Client disconnected." << endl;
            closesocket(clientSocket);
            break;
        } else {
            cerr << "recv() failed with error: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            break;
        }
        // cout << 
        // if (strlen(MSG) == 1) {
        if (MSG[0] == 'm') {
            FLAG = Listen;
            cout << "listen" << endl;
        } else if (MSG[0] == 'f') {
            FLAG = Request;
        } else if (MSG[0] == 's') {
            FLAG = Surf;
        } else if (MSG[0] == 'r') {
            FLAG = ReloadConfig;
        } else {
            FLAG = Exception;
        }
        // } 
        
        if (FLAG == Listen) {
            // Msg
            memset(recvBuf, '\0', sizeof(recvBuf));
            rtn = recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
            unsigned short clientPort = ntohs(clientAddr.sin_port);
            cout << "Received " << rtn << " bytes from client: " << "Port: " << clientPort << " msg: " << recvBuf << endl;
        } else if (FLAG == Request) {
            // File
            memset(recvBuf, '\0', sizeof(recvBuf));
            int bytesRead = recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
            if (bytesRead > 0) {
                string requestedFileName = recvBuf;
                string filePath = "D:\\code\\" + requestedFileName;
                // cout << filePath << endl;

                ifstream fileStream(filePath, ios::binary | ios::ate);
                if (fileStream.is_open()) {
                    streampos fileSize = fileStream.tellg();

                    fileStream.seekg(0, ios::beg);

                    char* fileBuffer = new char[fileSize];
                    cout << fileSize << endl;
                    fileStream.read(fileBuffer, fileSize);

                    send(clientSocket, fileBuffer, fileSize, 0);

                    delete[] fileBuffer;
                    fileStream.close();
                    cout << "Successfully transfer the requested file!\n";
                } else {
                    const char* errorResponse = "File not found.";
                    send(clientSocket, errorResponse, strlen(errorResponse), 0);
                }

                // closesocket(clientSocket);
            } else {
                closesocket(clientSocket);
                break;
            }
        } else if (FLAG == Surf) {


            memset(recvBuf, 0, sizeof(recvBuf));
            int bytesRead = recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
            if (bytesRead <= 0) {
                cerr << "Error receiving URL from client" << endl;
                closesocket(clientSocket);
                return;
            }

            std::string targetURL(recvBuf, bytesRead);
            cout << "Received URL: " << targetURL << endl;

            // 使用解析后的URL获取远程网页内容
            // 这里可以使用之前提供的方法来获取远程网页内容，并将其发送回客户端
            forwardRequest(clientSocket, targetURL);
        }
        
        // memset(recvBuf, '\0', sizeof(recvBuf));


        // char recvBuf[256];

    }
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }

Reload:
	loadConfig();
    if (_chdir(rootDirectory.c_str()) != 0) {
        cerr << "Failed to change directory to " << rootDirectory << endl;
        WSACleanup();
        return 1;
    }

    SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET) {
        cerr << "Error creating server socket" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    // int nRecvBuf=32*1024;//设置为32K
    // setsockopt(srvSocket,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(char));

    if (bind(srvSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Error binding server socket" << endl;
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }

    if (listen(srvSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Error listening on server socket" << endl;
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port " << serverPort <<  endl;

    vector<thread> clientThreads;

    while (true) {
        if (FLAG == ReloadConfig) {
            FLAG = 0;
            goto Reload;
        }
        
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "accept() failed with error: " << WSAGetLastError() << endl;
            continue;
        }

        cout << "Accepted a new client connection!" << endl;

        if (clientThreads.size() < MAX_CLIENTS) {
            thread clientThread(handleClient, clientSocket, clientAddr);
            clientThread.detach();
            clientThreads.push_back(move(clientThread));
        } else {
            cerr << "Maximum number of clients reached. Connection rejected." << endl;
            closesocket(clientSocket);
        }
    }

    closesocket(srvSocket);
    WSACleanup();

    return 0;
}
