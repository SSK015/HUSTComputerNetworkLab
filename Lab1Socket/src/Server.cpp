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

    std::string request = "GET " + targetURL.substr(hostEnd) + "index.html" + " HTTP/1.1\r\n";
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

std::string getContentType(const std::string& filePath) {
    if (filePath.find(".html") != std::string::npos) {
        return "text/html";
    } else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    } else if (filePath.find(".png") != std::string::npos) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

std::string createHttpResponse(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
    }

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 200 OK\r\n";
    responseStream << "Content-Type: " << getContentType(filePath) << "\r\n";
    responseStream << "Content-Length: " << file.tellg() << "\r\n\r\n";
    responseStream << file.rdbuf();

    return responseStream.str();
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
            char buffer[409600];
            // char request
            size_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead == -1) {
                std::cerr << "Error reading from client" << std::endl;
                closesocket(clientSocket);
                continue;
            }

            // 解析HTTP请求
            std::string request(buffer, bytesRead);
            size_t start = request.find("GET ");
            if (start == std::string::npos) {
                std::cerr << "Invalid HTTP request" << std::endl;
                closesocket(clientSocket);
                continue;
            }

            size_t end = request.find(" HTTP", start);
            if (end == std::string::npos) {
                std::cerr << "Invalid HTTP request" << std::endl;
                closesocket(clientSocket);
                continue;
            }

            // int rcvBufSize = 409600;
			// // printf("you want to set udp socket recv buff size to %d\n", rcvBufSize);
			// auto optlen = sizeof(rcvBufSize);
			// if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (const char*)rcvBufSize, optlen) < 0)
			// {
			// 	printf("setsockopt error=%d(%s)!!!\n", errno, strerror(errno));
			// 	// goto error;
			// }
			// char buffer[409600];

            std::string filePath = request.substr(start + 4, end - (start + 4));

            // 创建HTTP响应
            // filePath.append(rootDirectory);
            filePath = rootDirectory + filePath;
            cout << filePath << endl;
            std::string response = createHttpResponse(filePath);
            FILE* fq;
            if ((fq = fopen(filePath.c_str(), "rb")) == NULL) {
                printf("File not exist\n");
            }
            memset(buffer, '\0', sizeof(recvBuf));
            // bzero()
            while (!feof(fq)) {
                auto len = fread(buffer, 1, sizeof(buffer), fq);
                cout << len << endl;
                if (len != send(clientSocket, buffer, len, 0)) {
                    cout << "Writing" << endl;
                    break;
                }
            }

            fclose(fq);
            // 发送HTTP响应给客户端
            // int sendbytes = send(clientSocket, imageData.c_str(), imageData.length(), 0);
            // cout << sendbytes << endl;
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
