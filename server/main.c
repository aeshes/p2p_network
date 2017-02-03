#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include "define.h"
#include "iptable.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 666
#define MAX_CONNECTIONS 10


void WINAPI FormatError(DWORD error);
SOCKET WINAPI BindSocket(uint16_t port);
DWORD WINAPI HandleConnection(LPVOID sock);
void WINAPI PrintClientInfo(struct sockaddr_in *addr);
DWORD WINAPI RefreshNodeList(LPVOID);

int main(int argc, char *argv[])
{
	WSADATA wsadata;
	SOCKET server;
	SOCKET client;
	DWORD ThreadID;
	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	if (FAILED(WSAStartup(MAKEWORD(2, 0), &wsadata)))
	{
		FormatError(WSAGetLastError());
	}
	else
	{
		server = BindSocket(PORT);
		if (listen(server, MAX_CONNECTIONS) == SOCKET_ERROR)
		{
			FormatError(WSAGetLastError());
			WSACleanup();
			return -1;
		}

		while (TRUE)
		{
			client = accept(server, (struct sockaddr *)&client_addr, &addr_len);
			if (client != INVALID_SOCKET)
			{
				PrintClientInfo(&client_addr);
				CreateThread(NULL, 0, HandleConnection, &client, 0, &ThreadID);
			}
		}
	}
}

void WINAPI FormatError(DWORD errCode)
{
	char error[1000]; 
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		error, sizeof(error), NULL);
	printf("\nError: %s\n", error);
	getchar();
}

SOCKET WINAPI BindSocket(uint16_t port)
{
	SOCKET sock = INVALID_SOCKET;
	struct sockaddr_in local_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		FormatError(WSAGetLastError());
	}
	else
	{
		RtlZeroMemory(&local_addr, sizeof(local_addr));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(port);
		local_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
		{
			FormatError(WSAGetLastError());
		}
		else
		{
			printf("Server running...\n");
		}
	}

	return sock;
}

DWORD WINAPI HandleConnection(LPVOID sock)
{
	char buffer[PACKET_SIZE] = { 0 };

	SOCKET client = *(SOCKET *) sock;

	while (1)
	{
		int ret = recv(client, buffer, PACKET_SIZE, 0);
		if (ret == SOCKET_ERROR) break;
		printf("recieved...\n");

		if (ret > 0)
		{
			header *hdr = (header *) buffer;
			switch (hdr->command)
			{
				case ADD_NODE:
				{
					client_node node;
					memcpy(&node, &buffer[sizeof(header)], sizeof(client_node));
					add_node(&node);
					show_iptable();
					break;
				}
				case REM_NODE:
					break;
				case GET_LIST:
					send_iptable(client);
					break;
				default:
					printf("Unknown command\n");
					break;
			}
		}
	}

	closesocket(client);

	return 0;
}

void WINAPI PrintClientInfo(struct sockaddr_in *addr)
{
	char hostname[NI_MAXHOST];

	getnameinfo((struct sockaddr *)addr, sizeof(*addr), hostname, sizeof(hostname), NULL, 0, 0);
	printf("+ %s [%s] new connection\n", hostname, inet_ntoa(addr->sin_addr));
}
