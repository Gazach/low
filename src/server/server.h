#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    int stat_code;
    const char *status_line;
} stat_line;

static const stat_line status_code[] = {
    // 1xx Informational
    {100, "HTTP/1.1 100 Continue"},
    {101, "HTTP/1.1 101 Switching Protocols"},

    // 2xx Success
    {200, "HTTP/1.1 200 OK"},
    {201, "HTTP/1.1 201 Created"},
    {202, "HTTP/1.1 202 Accepted"},
    {203, "HTTP/1.1 203 Non-Authoritative Information"},
    {204, "HTTP/1.1 204 No Content"},
    {205, "HTTP/1.1 205 Reset Content"},
    {206, "HTTP/1.1 206 Partial Content"},

    // 3xx Redirection
    {300, "HTTP/1.1 300 Multiple Choices"},
    {301, "HTTP/1.1 301 Moved Permanently"},
    {302, "HTTP/1.1 302 Found"},
    {303, "HTTP/1.1 303 See Other"},
    {304, "HTTP/1.1 304 Not Modified"},
    {305, "HTTP/1.1 305 Use Proxy"},
    {307, "HTTP/1.1 307 Temporary Redirect"},

    // 4xx Client Error
    {400, "HTTP/1.1 400 Bad Request"},
    {401, "HTTP/1.1 401 Unauthorized"},
    {402, "HTTP/1.1 402 Payment Required"},
    {403, "HTTP/1.1 403 Forbidden"},
    {404, "HTTP/1.1 404 Not Found"},
    {405, "HTTP/1.1 405 Method Not Allowed"},
    {406, "HTTP/1.1 406 Not Acceptable"},
    {407, "HTTP/1.1 407 Proxy Authentication Required"},
    {408, "HTTP/1.1 408 Request Timeout"},
    {409, "HTTP/1.1 409 Conflict"},
    {410, "HTTP/1.1 410 Gone"},
    {411, "HTTP/1.1 411 Length Required"},
    {412, "HTTP/1.1 412 Precondition Failed"},
    {413, "HTTP/1.1 413 Request Entity Too Large"},
    {414, "HTTP/1.1 414 Request-URI Too Long"},
    {415, "HTTP/1.1 415 Unsupported Media Type"},
    {416, "HTTP/1.1 416 Requested Range Not Satisfiable"},
    {417, "HTTP/1.1 417 Expectation Failed"},

    // 5xx Server Error
    {500, "HTTP/1.1 500 Internal Server Error"},
    {501, "HTTP/1.1 501 Not Implemented"},
    {502, "HTTP/1.1 502 Bad Gateway"},
    {503, "HTTP/1.1 503 Service Unavailable"},
    {504, "HTTP/1.1 504 Gateway Timeout"},
    {505, "HTTP/1.1 505 HTTP Version Not Supported"}
};

static inline const char* lookup_status_line(int code) {
    size_t count = sizeof(status_code) / sizeof(status_code[0]);
    for (size_t i = 0; i < count; i++) {
        if (status_code[i].stat_code == code) {
            return status_code[i].status_line;
        }
    }
    return NULL;
}

// Call api
int server(const char *port);

// Macro wrapper
#define STATUS_LINE lookup_status_line

#endif