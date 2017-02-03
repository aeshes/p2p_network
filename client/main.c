#include <stdio.h>
#include "define.h"
#include "iptable.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 666


SOCKET connect_to_server(char *host, uint16_t port);
void get_nodes_from_server(void);
void send_packet(SOCKET sock, uint32_t command, const char *data, size_t size_of_data);
void send_fake_nodes(SOCKET sock);


int main(int argc, char *argv[])
{
	WSADATA wsa;
	HANDLE hMutex = NULL;

	CreateMutex(NULL, TRUE, "muu===");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		goto _cleanup;

	if (FAILED(WSAStartup(MAKEWORD(2, 2), &wsa)))
	{
		printf("Error while initializing WinSock 2 library\n");
		goto _cleanup;
	}

	get_nodes_from_server();

_cleanup:
	WSACleanup();
	CloseHandle(hMutex);
	exit(0);
}

SOCKET connect_to_server(char *host, uint16_t port)
{
	SOCKET server_socket;
	struct sockaddr_in server_addr;

	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_HOST);
	server_addr.sin_port = htons(SERVER_PORT);

	int ret = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret != 0)
	{
		printf("Error while connecting to server: %d\n", WSAGetLastError());
		closesocket(server_socket);
		return INVALID_SOCKET;
	}
	return server_socket;
}

void send_packet(SOCKET sock, uint32_t command, const char *data, size_t size_of_data)
{
	char packet[PACKET_SIZE] = { 0 };
	header hdr = { 0 };

	hdr.command = command;
	memcpy(packet, &hdr, sizeof(header));
	memcpy(&packet[sizeof(header)], data, size_of_data);

	send(sock, packet, PACKET_SIZE, 0);
}

void send_fake_nodes(SOCKET sock)
{
	for (int i = 0; i < 5; ++i)
	{
		client_node node;
		node.ip = rand() % 65000;
		node.port = rand() % 0xFFFF;
		node.time = time(0);

		send_packet(sock, ADD_NODE, (char *)&node, sizeof(node));
		printf("packet sent\n");
		Sleep(1000);
	}
}

void get_nodes_from_server(void)
{
	SOCKET server_socket;
	char packet[PACKET_SIZE] = { 0 };
	unsigned long int nonblocking = 1;
	int len = 0;

	server_socket = connect_to_server(SERVER_HOST, SERVER_PORT);
	if (server_socket == INVALID_SOCKET)
		goto _cleanup;

	send_fake_nodes(server_socket);
	send_packet(server_socket, GET_LIST, NULL, 0);

	ioctlsocket(server_socket, FIONBIO, &nonblocking);
	do
	{
		if ((len = recv(server_socket, packet, PACKET_SIZE, 0)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				return;
			else
				len = 1;
		}
		else
		{
			client_node node;
			memcpy(&node, &packet[sizeof(header)], sizeof(client_node));
			add_node(&node);
		}
	}while (len > 0);

	show_iptable();

_cleanup:
	closesocket(server_socket);
}
