#include "server.h"
#include "iptable.h"
#include <stdio.h>


SOCKET connect_to_server(char *host, uint16_t port)
{
	SOCKET server_socket;
	struct sockaddr_in server_addr;

	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(port);

	int ret = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret != 0)
	{
		printf("Error while connecting to server: %d\n", WSAGetLastError());
		closesocket(server_socket);
		return INVALID_SOCKET;
	}
	return server_socket;
}

// Sends packet to the server over TCP. This function sets 'command'
// field of a packet and transmits data.
static void send_to_server(SOCKET sock, uint32_t command, void *data, size_t size_of_data)
{
	char packet[PACKET_SIZE] = { 0 };
	header hdr = { 0 };

	hdr.command = command;
	memcpy(packet, &hdr, sizeof(header));
	memcpy(&packet[sizeof(header)], data, size_of_data);

	send(sock, packet, PACKET_SIZE, 0);
}

void register_on_server(SOCKET server, client_node *mynode)
{
	send_to_server(server, ADD_NODE, mynode, sizeof(client_node));
}

void get_nodes_from_server(SOCKET server_socket)
{
	char packet[PACKET_SIZE] = { 0 };
	unsigned long int nonblocking = 1;
	int len = 0;

	//send_fake_nodes(server_socket);
	send_to_server(server_socket, GET_LIST, NULL, 0);

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

