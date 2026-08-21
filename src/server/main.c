#include <stdio.h>

// Macro
#ifdef _WIN32 // If program compiled on windows
#include <windows.h>

#define sleep_ms(ms) Sleep(ms)

#else // if program compiled on unix system
#include <unistd.h>

#define sleep_ms(ms) usleep((ms) * 1000)

#endif

// Server main entries

int main(){
    // server loop
    while(1){
        // get request
        printf("Wait for request...\n");
        sleep_ms(1000);
    }
    
    return 0;
}