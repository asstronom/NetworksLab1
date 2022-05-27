#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;

int main(){
	//creating WSADATA object
	WSADATA wsaData;
	//initializing wsaData
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//initializing addrinfo structures
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	//Resolving local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	//creating listen socket
	SOCKET ListenSocket = INVALID_SOCKET;
	//initialize listen socket with 'socket' function
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//check for errors to ensure that the socket is valid
	if (ListenSocket == INVALID_SOCKET) {
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	cout << "Created listen socket" << endl;

	//Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	//check for errors
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Binded" << endl;
	//free addrinfo because it is no longer needed
	freeaddrinfo(result);

	//listen on the socket
	//SOMAXCONN - tells winsock to allow maximum reasonable number of pending connections in the queue
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Started listening" << endl;

	//creating temporary socket for accepting connections from client
	SOCKET ClientSocket = INVALID_SOCKET;

	cout << "Waiting to accept client..." << endl;

	//accepting a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed: ", WSAGetLastError();
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Accepted connection!" << endl;

	//receive buffer 512 bytes
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iSendResult = 0;
	iResult = 0;

	char firstreceive[9] = "";
	float secondreceive = 0.0f;

	//Receive until peer shuts down the connection
	do {
		//receiving data
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			cout << "Bytes received: " << iResult << endl;
			unsigned int i = 0;
			for (i; i < 8; i++) {
				firstreceive[i] = recvbuf[i];
			}
			union {
				float a;
				unsigned char bytes[sizeof(float)];
			} thing;

			for (i = 0; i < sizeof(float); i++) {
				thing.bytes[i] = recvbuf[i+8];
			}
			secondreceive = thing.a;

			cout << "8 chars: " << firstreceive << endl;
			cout << "1 float: " << secondreceive << endl;

			//echo data to client
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				cout << "send failed " << WSAGetLastError() << endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			cout << "Bytes send " << iSendResult << endl;
		}
		//close connection if received 0 bytes
		else if (iResult == 0)
			cout << "Connection closing..." << endl;
		else {
			cout << "recv failed: " << WSAGetLastError() << endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;

}