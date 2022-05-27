#include <iostream>
#include <winsock2.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 8888

#define CHARMESSLEN 8
#define FLOATMESLEN 1

using namespace std;

int main()
{


    system("title UDP Server");

    sockaddr_in server, client;

    // initialise winsock
    WSADATA wsa;
    printf("Initialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d", WSAGetLastError());
        exit(0);
    }
    printf("Initialised.\n");

    // create a socket
    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket: %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done.");

    printf("Waiting for data...");
    fflush(stdout);
    char message[BUFLEN] = {};

    // try to receive some data, this is a blocking call
    int message_len;
    int slen = sizeof(sockaddr_in);
    if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
    {
        printf("recvfrom() failed with error code: %d", WSAGetLastError());
        exit(0);
    }

    // print details of the client/peer and the data received
    printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    string charmsg = "";
    for (int i = 0; i < CHARMESSLEN; i++) {
        charmsg += message[i];
    }
    union {
        float a;
        char bytes[4];
    } thing;
    for (int i = 0; i < sizeof(float) * FLOATMESLEN; i++) {
        thing.bytes[i] = message[i + CHARMESSLEN];
    }
    cout << "8 chars: " << charmsg << endl;
    cout << "1 float: " << thing.a << endl;


    closesocket(server_socket);
    WSACleanup();
}