#include <stdio.h>
#include "define.h"
#include "iptable.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 666


SOCKET connect_to_server(char *host, uint16_t port);
void register_on_server(SOCKET server, client_node *mynode);
void get_nodes_from_server(SOCKET server);
void send_packet(SOCKET sock, uint32_t command, const char *data, size_t size_of_data);
void send_fake_nodes(SOCKET sock);
void send_my_info(SOCKET sock);
SOCKET bind_udp_sock(client_node *myinfo);
DWORD WINAPI handle_udp_packets(void *sock);


int main(int argc, char *argv[])
{
	WSADATA wsa;
	HANDLE hMutex = NULL;
	SOCKET server;
	SOCKET udp_listener;
	client_node my_node;

	CreateMutex(NULL, TRUE, "muu===");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		goto _cleanup;

	if (FAILED(WSAStartup(MAKEWORD(2, 2), &wsa)))
	{
		printf("Error while initializing WinSock 2 library\n");
		goto _cleanup;
	}

	server 	= connect_to_server(SERVER_HOST, SERVER_PORT);
	if (server != INVALID_SOCKET)
	{
		udp_listener = bind_udp_sock(&my_node);
		register_on_server(server, &my_node);
		get_nodes_from_server(server);
		shutdown(server, 0);

		DWORD thID = 0;
		CreateThread(NULL, 0, handle_udp_packets, &udp_listener, 0, &thID);
	}

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

void register_on_server(SOCKET server, client_node *mynode)
{
	send_packet(server, ADD_NODE, (char *)mynode, sizeof(client_node));
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
		node.port = htons(rand() % 0xFFFF);
		node.time = time(0);

		send_packet(sock, ADD_NODE, (char *)&node, sizeof(node));
		printf("packet sent\n");
		Sleep(1000);
	}
}

void get_nodes_from_server(SOCKET server_socket)
{
	char packet[PACKET_SIZE] = { 0 };
	unsigned long int nonblocking = 1;
	int len = 0;

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
}

static void get_local_ip(char *ip_str)
{
	char hostname[255] = { 0 };
	PHOSTENT hostinfo = NULL;
	char *ip = NULL;

	if (gethostname(hostname, sizeof(hostname)) == 0)
	{
		if ((hostinfo = gethostbyname(hostname)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]);
			strcpy(ip_str, ip);
		}
	}
}

SOCKET bind_udp_sock(client_node *myinfo)
{
	SOCKET sock;
	SOCKADDR_IN local_addr;
	char ip[50] = { 0 };

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error while creating UDP socket: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	srand(time(0));
	local_addr.sin_family 		= AF_INET;
	local_addr.sin_addr.s_addr 	= INADDR_ANY;
	local_addr.sin_port 		= htons(rand() % 0xAA00 + 1024);

	if (bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)) == SOCKET_ERROR)
	{
		printf("Error while binding udp socket: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}
	get_local_ip(ip);

	myinfo->ip 		= inet_addr(ip);
	myinfo->port 	= local_addr.sin_port;
	myinfo->time 	= time(0);

	return sock;
}

/*
	Recieves packets from peers using listening udp socket.
	Performs actions declared in the 'command' field of header.
*/
DWORD WINAPI handle_udp_packets(void *listener)
{
	SOCKET sock = *(SOCKET *) listener;
	struct sockaddr_in client_addr;
	char packet[PACKET_SIZE];

	while (1)
	{
		int client_addr_size = sizeof(client_addr);
		int size = recvfrom(sock,
			packet,
			PACKET_SIZE,
			0,
			(struct sockaddr *)&client_addr,
			&client_addr_size);
		if (size == SOCKET_ERROR)
		{
			printf("recvfrom() error: %d\n", WSAGetLastError());
			continue;
		}

		header *hdr = (header *) packet;
		switch(hdr->command)
		{
			case GET_LIST:
				printf("GET_LIST command\n");
				break;
			default:
				printf("OTHER COMMAND\n");
				break;
		}
	}
	return 0;
}
