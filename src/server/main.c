#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific networking headers and macros
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#define sleep_ms(ms) Sleep(ms)
#else // Unix
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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
#define shutdownsocket(s, h) shutdown(s, h)
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

#define PORT "5000"
#define BACKLOG 64 /* for listen() */

struct addrinfo *GetAddressInfo(const char *port) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return NULL;
    }
    return res;
}

int CreateSocket(struct addrinfo *res) {
    printf("Creating a Socket\n");
    return socket(res->ai_family, res->ai_socktype, res->ai_protocol);
}

int BindSocket(int cSocket, struct addrinfo *res) {
    return bind(cSocket, res->ai_addr, res->ai_addrlen);
}

int accepting_shit(int cSocket){
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    int new_fd;
    
    addr_size = sizeof their_addr;
    
    new_fd = accept(cSocket, (struct sockaddr *)&their_addr, &addr_size);
    
    return new_fd;
}

const char *methodCheck(const char *getting_req) {
    if (strncmp(getting_req, "GET ", 4) == 0) {
        printf("[MATCH] Method received : GET\n");
        return "GET";
    }  else   if (strncmp(getting_req, "POST ", 5) == 0) {
        printf("[MATCH] Method received : POST\n");
        return "POST";
    } 

    return NULL;
}
// Server main entry
int main(int argc, char *argv[]) {
    net_init();

    struct addrinfo *res = GetAddressInfo(PORT);
    if (res == NULL) {
        return 1;
    }

    SOCKET socket_desc = CreateSocket(res);
    if (socket_desc == INVALID_SOCKET) {
        printf("Could not create socket\n");
        freeaddrinfo(res);
        return 1;
    }

    int bindResult = BindSocket(socket_desc, res);
    freeaddrinfo(res);  // done with this either way
    if (bindResult < 0) {
        perror("bind failed.");
        return 1;
    }

    listen(socket_desc, BACKLOG);
    
    while(1){
        int new_fd = accepting_shit(socket_desc);
        if (new_fd == INVALID_SOCKET) {
            perror("accept failed");
            continue; // don't process this connection at all
        }
        
        int max_leng = 100000;
        char getting_req[max_leng];
        memset(getting_req, 0, max_leng);
        
        int num_byte = recv(new_fd, getting_req, max_leng, 0);
        printf("[DEBUG] recv returned: %d\n", num_byte);
        printf("[DEBUG] buffer contents: \"%s\"\n", getting_req);
        
        if (num_byte <= 0) {
            printf("Client disconnected or recv error.\n");
            closesocket(new_fd);
            continue; // go back to accept() for the next connection
        }
        
        const char *method = methodCheck(getting_req);
        
        if (method == NULL){
            printf("Method Not found.\n");
            return 1;
        } else {
            printf("%s", method);
        }
        
        const char *body = "<html><body><h1>Hello from my C server!</h1></body></html>";
        char response[1024];
        
        int response_len = snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(body), body);
        
        send(new_fd, response, response_len, 0);
        closesocket(new_fd);
    }


    net_cleanup();
    closesocket(socket_desc);
    return 0;
}