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
enum {Request, Listen, Surf, Exception};
string serverAddress = "127.0.0.1";
int serverPort = 67;
string rootDirectory = "/";
// string root

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
    int FLAG =0;
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
        } else {
            FLAG = Exception;
        }
        // } 
        
        if (FLAG == Listen) {
            // Msg
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

        } else {
            cout << "error happend" << endl;
        }
        
        // memset(recvBuf, '\0', sizeof(recvBuf));


        // char recvBuf[256];

    }
}

void forwardRequest(int clientSocket, const std::string& targetURL) {
    size_t hostStart = targetURL.find("://");
    if (hostStart == std::string::npos) {
        std::cerr << "Invalid URL format" << std::endl;
        return;
    }
    hostStart += 3; // skip "://" 
    size_t hostEnd = targetURL.find('/', hostStart);
    std::string host = targetURL.substr(hostStart, hostEnd - hostStart);

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

    struct sockaddr_in targetAddr;
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(80);
    targetAddr.sin_addr = *((struct in_addr*)targetHost->h_addr);

    if (connect(targetSocket, (struct sockaddr*)&targetAddr, sizeof(targetAddr)) < 0) {
        perror("Error connecting to target");
        closesocket(targetSocket);
        return;
    }

    std::string request = "GET " + targetURL.substr(hostEnd) + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n\r\n";

    int bytesSent = send(targetSocket, request.c_str(), request.length(), 0);
    if (bytesSent < 0) {
        perror("Error sending request to target");
        closesocket(targetSocket);
        return;
    }

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    while (true) {
        int bytesRead = recv(targetSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            break;
        }

        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent < 0) {
            perror("Error sending response to client");
            break;
        }
    }

    closesocket(targetSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }

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
