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
        
        if (num_byte <= 0) {
            printf("Client disconnected or recv error.\n");
            closesocket(new_fd);
            continue; // go back to accept() for the next connection
        }
        
        const char *method = methodCheck(getting_req);
        const char *response_body;
        const char *status_line;
        
        if (method == NULL) {
            printf("Method Not found.\n");
            status_line = "HTTP/1.1 400 Bad Request";
            response_body = "<html><body><h1>400 Bad Request</h1></body></html>";
        } else if (strcmp(method, "GET") == 0) {
            printf("[HANDLING] GET request\n");
            status_line = "HTTP/1.1 200 OK";
            response_body = "<html><body><h1>Hello, this was a GET</h1></body></html>";
        } else if (strcmp(method, "POST") == 0) {
            printf("[HANDLING] POST request\n");
            // Find end of headers
            char *body_start = strstr(getting_req, "\r\n\r\n");
            
            if (body_start == NULL) {
                printf("No header/body separator found (request incomplete?)\n");
            } else {
                body_start += 4;
            
                // Find Content-Length header
                long content_length = 0;
                char *cl_header = strstr(getting_req, "Content-Length:");
                if (cl_header != NULL) {
                    content_length = strtol(cl_header + strlen("Content-Length:"), NULL, 10);
                }
            
                // Reject garbage/negative values outright
                if (content_length < 0) {
                    content_length = 0;
                }
            
                // Clamp to what the buffer can actually hold, leaving room for
                // the null terminator we write below.
                long max_body = max_leng - (body_start - getting_req) - 1;
                if (content_length > max_body) {
                    printf("[WARN] Content-Length %ld exceeds buffer capacity, clamping to %ld\n",
                           content_length, max_body);
                    content_length = max_body;
                }
            
                // How much body did we already get in this recv()
                long body_have = num_byte - (body_start - getting_req);
            
                // If we haven't received the full body yet, keep recv'ing
                while (body_have < content_length) {
                    int space_left = max_leng - num_byte;
                    if (space_left <= 0) break; // buffer full, stop to avoid overflow
            
                    int more = recv(new_fd, getting_req + num_byte, space_left, 0);
                    if (more <= 0) break; // connection closed/error
                    num_byte += more;
                    body_have += more;
                }
            
                // Null-terminate right after the body ends, for safe printing
                body_start[content_length] = '\0';
            
                printf("[POST BODY] %s\n", body_start);
            }
            status_line = "HTTP/1.1 200 OK";
            response_body = "<html><body><h1>Got your POST data!</h1></body></html>";
        } else {
            // method matched something you didn't expect (shouldn't happen given methodCheck, but safe default)
            status_line = "HTTP/1.1 501 Not Implemented";
            response_body = "<html><body><h1>501 Not Implemented</h1></body></html>";
        }
        
        char response[2048];
        int response_len = snprintf(response, sizeof(response),
            "%s\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            status_line, strlen(response_body), response_body);
        
        send(new_fd, response, response_len, 0);
        closesocket(new_fd);
    }


    net_cleanup();
    closesocket(socket_desc);
    return 0;
}