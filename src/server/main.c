#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific networking headers and macros
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define sleep_ms(ms) Sleep(ms)
#else // Unix
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif


// POSIX doesn't have Winsock's SOCKET type or closesocket() —
// give Unix builds the same names so the rest of the codebase
// doesn't need to branch on platform.
#ifndef _WIN32
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket(s) close(s)
#endif

// Networking init/cleanup. Winsock needs WSAStartup/WSACleanup;
// POSIX sockets need no equivalent, so the Unix side is a no-op.
#ifdef _WIN32
void net_init(void) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

void net_cleanup(void) {
    WSACleanup();
}
#else
void net_init(void) {}
void net_cleanup(void) {}
#endif

#define PORT 5000
#define BACKLOG 64 /* for listen() */

int CreateSocket(void){
    SOCKET cSocket;
    printf("Creating a Socket\n");
    
    cSocket = socket(AF_INET, SOCK_STREAM, 0);
    return cSocket; 
}

int BindSocket(int cSocket){
    int iRetval=-1;
    struct sockaddr_in  remote= {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(PORT); /* Local port */
    
    iRetval = bind(cSocket,(struct sockaddr *)&remote,sizeof(remote));
    return iRetval;
}

// Server main entry
int main(int argc, char *argv[]) {
    net_init();    

    int socket_desc, sock, clientLen, read_size;

    // Making Socket
    socket_desc = CreateSocket();
    if (socket_desc == -1) // if failed return exit.
    {
        printf("Could not create socket");
        return 1;
    }
    printf("Socket created\n");
    
    // bind the created socket
    if( BindSocket(socket_desc) < 0)
    {
        //print the error message
        perror("bind failed.");
        return 1;
    }
    printf("bind done\n");
    
    // listening shit
    listen(socket_desc, BACKLOG);
    printf("This mf listening!\n");
    
    // server loop
    while(1){
        // get request
        printf("Wait for request...\n");
        sleep_ms(1000);
    }
    
    net_cleanup();

    return 0;
}