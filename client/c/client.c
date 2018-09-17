#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCKAPI_

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <stdlib.h>
#include <stdio.h>

const char *socket_path = "server.sock";

int start_winsock();
void stop_winsock();
int with_winsock();
int client(SOCKET sock);

int __cdecl main()
{
    int error = start_winsock();
    if (error) return error;

    error = with_winsock();

    stop_winsock();

    printf("client PID: %u\n", GetCurrentProcessId());

    return error;
}

int start_winsock()
{
    WSADATA data;
    int error = WSAStartup(MAKEWORD(2, 2), &data);
    if (error)
    {
        printf("WSAStartup error: %d\n", error);
    }

    return error;
}

void stop_winsock()
{
    WSACleanup();
}

int with_winsock()
{
    SOCKET sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        printf("socket error: %d", error);
        return error;
    }

    int error = client(sock);

    closesocket(sock);

    return error;
}

int client(SOCKET sock)
{
    struct sockaddr_un server_addr = {0};
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path));

    printf("client: connecting to %s\n", server_addr.sun_path);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        int error = WSAGetLastError();
        printf("connect error: %d", error);
        return error;
    }

    struct sockaddr_un peer_addr = {0};
    socklen_t namelen = sizeof(peer_addr);
    memset(&peer_addr, 0, sizeof(peer_addr));
    if (getpeername(sock, (struct sockaddr *)&peer_addr, &namelen) == -1)
    {
        int error = WSAGetLastError();
        printf("getpeername error: %d\n", error);
        return error;
    }

    printf("getpeername returned address: %s, size: %d\n", peer_addr.sun_path, namelen);

    char buf[100];
    memset(buf, 0, sizeof(buf));
    int received = recv(sock, buf, sizeof(buf), 0);
    if (received == -1)
    {
        int error = WSAGetLastError();
        printf("recv error: %d\n", error);
        return error;
    }

    printf("received: %d bytes, %s\n", received, buf);

    return 0;
}
