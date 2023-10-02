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
    while (true) {
        memset(recvBuf, '\0', sizeof(recvBuf));
        int rtn = recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
        if (rtn > 0) {
			unsigned short clientPort = ntohs(clientAddr.sin_port);
            cout << "Received " << rtn << " bytes from client: " << "Port: " << clientPort << " msg: " << recvBuf << endl;
        } else if (rtn == 0) {
            cout << "Client disconnected." << endl;
            closesocket(clientSocket);
            break;
        } else {
            cerr << "recv() failed with error: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            break;
        }
        // char recvBuf[256];
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
    }
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
