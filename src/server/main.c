#include <stdio.h>
#include <stdlib.h>

// Macro
#ifdef _WIN32 // If program compiled on windows
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>  
#include <ws2tcpip.h>
#include <windows.h>

#define sleep_ms(ms) Sleep(ms)

#else // if program compiled on unix system
#include <unistd.h>

#define sleep_ms(ms) usleep((ms) * 1000)

#endif

// init windows
#ifdef _WIN32

void init_wins(void) {
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

#endif

// Server main entries

int main(){
    
    #ifdef _WIN32
        init_wins();
    #endif
    
    // server loop
    while(1){
        // get request
        printf("Wait for request...\n");
        sleep_ms(1000);
    }
    
    
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    return 0;
}