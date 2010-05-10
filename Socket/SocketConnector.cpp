#include "StdAfx.h"
#include "SocketConnector.h"

SocketConnector::SocketConnector(CRITICAL_SECTION criticalSection, Head* headPar)
{
	headCriticalSection = criticalSection;
	head = headPar;





	initializeConnector();
	closeConnector();


}

int SocketConnector::initializeConnector(){
	
	ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
	result = NULL;
	recvbuflen = DEFAULT_BUFLEN;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
	printf("Waiting for connection...\n");
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
printf("Here12\n");
    // No longer need server socket
    closesocket(ListenSocket);
printf("Here13\n");
	int sentCount=0;
    // Receive until the peer shuts down the connection
    do {
		
	printf("Here\n");
       /* iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {*/
		if(1){
			static int counter = 0;
            printf("(%d) Bytes received: %d\n", counter++ , iResult);

        // Echo the buffer back to the sender
			//EnterCriticalSection(&headCriticalSection);
			sprintf(sentbuf, "%lf %lf %lf %lf %lf %lf",head->x, head->y, head->z, 0.3f, 0.4f, 0.5f);
			printf("%lf\n", head->x);
			//LeaveCriticalSection(&headCriticalSection);


			iSendResult = send( ClientSocket, sentbuf, strlen(sentbuf), 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
			Sleep(15);
            printf("%d Bytes sent: %d\n",sentCount++, iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 0;
        }

    } while (1);//(iResult > 0);
	return 0;
}
int SocketConnector::closeConnector(){
	// shutdown the connection since we're done
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


SocketConnector::~SocketConnector(void)
{
}
