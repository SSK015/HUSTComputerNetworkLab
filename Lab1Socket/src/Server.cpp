#include <iostream>
// #include <ios>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <winsock2.h>
#include <thread>
#include <limits>
#include <sstream>
#include <regex>
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
int ResType = 0;
// string root
// std::vector<

void LoadResources(string path, string& PicName, string&CssName) {
    std::ifstream inputFile("D:/code/index.html");
    std::string htmlContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

    std::regex hrefRegex("href\\s*=\\s*\"([^\"]*)\"");
    std::regex srcRegex("src\\s*=\\s*\"([^\"]*)\"");

    std::smatch match;
    std::smatch match1;

    std::cout << "Matching href attributes:" << std::endl;
    while (std::regex_search(htmlContent, match, hrefRegex)) {
        std::cout << "href: " << match[1] << std::endl;
        CssName = match[1];
        htmlContent = match.suffix().str();
    }
    std::ifstream inputFile1("D:/code/index.html");
    std::string htmlContent1((std::istreambuf_iterator<char>(inputFile1)), std::istreambuf_iterator<char>());

    // htmlContent1 = std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    std::cout << "\nMatching src attributes:" << std::endl;
    while (std::regex_search(htmlContent1, match1, srcRegex)) {
        std::cout << "src: " << match1[1] << std::endl;
        PicName = match1[1];
        htmlContent1 = match1.suffix().str();
    }

    // return 0;
}



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
        ResType = 1;
        return "text/html";
    } else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) {
        ResType = 0;
        return "image/jpeg";
        // ResType = 0;
    } else if (filePath.find(".png") != std::string::npos) {
        ResType = 0;
        return "image/png";
        // ResType = 0;
    } else {
        ResType = 0;
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
    file.seekg(0, std::ios::end);
    responseStream << "Content-Length: " << file.tellg() << "\r\n\r\n";
    file.seekg(0, std::ios::beg);
    responseStream << file.rdbuf();

    // cout << responseStream.str() << endl;
    file.close();
    return responseStream.str();

}

std::string create403HttpResponse(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
    }

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 403 Forbidden\r\n";
    responseStream << "Content-Type: " << getContentType(filePath) << "\r\n";
    file.seekg(0, std::ios::end);
    responseStream << "Content-Length: " << file.tellg() << "\r\n\r\n";
    file.seekg(0, std::ios::beg);
    responseStream << file.rdbuf();

    // cout << responseStream.str() << endl;
    file.close();
    return responseStream.str();

}

std::string create404HttpResponse(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
    }

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 404 Forbidden\r\n";
    responseStream << "Content-Type: " << getContentType(filePath) << "\r\n";
    file.seekg(0, std::ios::end);
    responseStream << "Content-Length: " << file.tellg() << "\r\n\r\n";
    file.seekg(0, std::ios::beg);
    responseStream << file.rdbuf();

    // cout << responseStream.str() << endl;
    file.close();
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
        loadConfig();
        
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
            // char buffer[409600];
            char* buffer = new char[4000001];
            // char* fileBuf = new char[4000001];
            // char request
            // size_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            size_t bytesRead = recv(clientSocket, buffer, 4000001, 0);
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
            FILE* fq;
            // filePath.append(rootDirectory);
            if (filePath == "notpermit.txt") {
                printf("File not exist\n");
                std::string response403 = create403HttpResponse("403.html");
                int sendbytes = send(clientSocket, response403.c_str(), response403.length(), 0);
                string filePath = "D:\\code\\403.html";
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
                    cout << "403 file!\n";
                } 

                continue;
            }
            filePath = rootDirectory + filePath;
            cout << filePath << endl;
            
            
            if ((fq = fopen(filePath.c_str(), "rb")) == NULL) {
                printf("File not exist\n");
                std::string response404 = create404HttpResponse("404.html");
                int sendbytes = send(clientSocket, response404.c_str(), response404.length(), 0);
                string filePath = "D:\\code\\404.html";
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
                    cout << "404 file!\n";
                } 

                continue;
            }
            std::string CssName, PicName;

            LoadResources(filePath, CssName, PicName);
            std::string response = createHttpResponse(filePath);
            int sendbytes = send(clientSocket, response.c_str(), response.length(), 0);
            cout << sendbytes << endl;
            // FILE* fq;
            if ((fq = fopen(filePath.c_str(), "rb")) == NULL) {
                printf("File not exist\n");
            }
            // memset(buffer, '\0', sizeof(buffer));
            memset(buffer, '\0', 4000001);
            fseek(fq, 0 ,SEEK_END);
            int fileSize = ftell(fq);
            fseek(fq, 0 ,SEEK_SET);
            // cout << fileSize << endl;
            // bzero()
            while (!feof(fq)) {
                auto len = fread(buffer, 1, fileSize, fq);
                cout << len << endl;
                if (len != send(clientSocket, buffer, len, 0)) {
                    cout << "Writing" << endl;
                    break;
                }
            }

            fclose(fq);
            cout << ResType << endl;
            // break;
            if (ResType) {
                if ((fq = fopen(CssName.c_str(), "rb")) == NULL) {
                    printf("File not exist\n");
                }
                // memset(buffer, '\0', sizeof(buffer));
                memset(buffer, '\0', 4000001);
                fseek(fq, 0 ,SEEK_END);
                fileSize = ftell(fq);
                fseek(fq, 0 ,SEEK_SET);
                // cout << fileSize << endl;
                // bzero()
                while (!feof(fq)) {
                    auto len = fread(buffer, 1, fileSize, fq);
                    cout << len << endl;
                    if (len != send(clientSocket, buffer, len, 0)) {
                        cout << "Writing" << endl;
                        break;
                    }
                }

                fclose(fq);

                if ((fq = fopen(PicName.c_str(), "rb")) == NULL) {
                    printf("File not exist\n");
                }
                // memset(buffer, '\0', sizeof(buffer));
                memset(buffer, '\0', 4000001);
                fseek(fq, 0 ,SEEK_END);
                fileSize = ftell(fq);
                fseek(fq, 0 ,SEEK_SET);
                // cout << fileSize << endl;
                // bzero()
                while (!feof(fq)) {
                    auto len = fread(buffer, 1, fileSize, fq);
                    cout << len << endl;
                    if (len != send(clientSocket, buffer, len, 0)) {
                        cout << "Writing" << endl;
                        break;
                    }
                }

                fclose(fq);
            }

            
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
