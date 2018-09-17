#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCKAPI_

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <stdlib.h>
#include <stdio.h>

const char socket_path[] = "server.sock";

int start_winsock();
void stop_winsock();
int with_winsock();
int server(SOCKET sock);

int __cdecl main(void) {
    int error = start_winsock();
    if (error) return error;

    error = with_winsock();

    stop_winsock();
    return 0;
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
    SOCKET sock = INVALID_SOCKET;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        int error = WSAGetLastError();
        printf("socket error: %d\n", error);
        return error;
    }

    int error = server(sock);

    DeleteFileA(socket_path); // analogous to 'unlink'
    return error;
}

int server(SOCKET sock)
{
    const char buf[] = "af_unix from Windows to Windows!";

    SOCKADDR_UN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(socket_path) - 1);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        printf("bind error: %d", error);
        return error;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        printf("listen error: %d", error);
        return error;
    }

    printf("accepting connections on: '%s'\n", socket_path);
    SOCKET client = accept(sock, NULL, NULL);
    if (client == INVALID_SOCKET) {
        int error = WSAGetLastError();
        printf("accept error: %d", error);
        return error;
    }

    printf("accepted a connection\n" );

    printf("relaying %zu bytes: '%s'\n", sizeof(buf) - 1, buf);
    if (send(client, buf, (int)sizeof(buf) - 1, 0 ) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        printf("send error: %d", error);
        return error;
    }

	ULONG pid;
	if (ioctlsocket(client, SIO_AF_UNIX_GETPEERPID, &pid) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        printf("WSAIoctl(SIO_AF_UNIX_GETPEERPID) error: %d", error);
        return error;
	}

	printf("peer PID: %u\n", pid);

    if (shutdown(client, 0) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        printf("shutdown error: %d", error);
        return error;
    }

    return 0;
}