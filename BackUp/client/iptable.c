#include "iptable.h"

#include <stdlib.h>
#include <string.h>


#define NODE_TABLE_SIZE 255

static const client_node null_node = { 0 };
static client_node routing_table[NODE_TABLE_SIZE] = { 0 };
static size_t node_ptr = 0;
static size_t node_count = 0;

static client_node my_node;


static client_node * node_already_exists(client_node * node)
{
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (routing_table[node_idx].ip == node->ip && routing_table[node_idx].port == node->port)
			return &routing_table[node_idx];
	}
	return NULL;
}

static int compare_time_asc(const void *x, const void *y)
{
	return ((client_node *)x)->time - ((client_node *)y)->time;
}

static int compare_time_desc(const void *x, const void *y)
{
	return ((client_node *)y)->time - ((client_node *)x)->time;
}

static void sort_by_time_asc(void)
{
	qsort(routing_table, node_count, sizeof(client_node), compare_time_asc);
}

static void sort_by_time_desc(void)
{
	qsort(routing_table, node_count, sizeof(client_node), compare_time_desc);
}

void add_node(client_node * new_node)
{
	client_node *tmp = NULL;

	if ((tmp = node_already_exists(new_node)) != NULL)
	{
		tmp->time = new_node->time;
	}
	else
	{
		routing_table[node_ptr++] = *new_node;
		node_ptr %= NODE_TABLE_SIZE;
		if (node_count < NODE_TABLE_SIZE)
			++node_count;
	}
	sort_by_time_asc();
}

void set_my_node(client_node * mynode)
{
	my_node = *mynode;
}

void send_iptable(SOCKET sock)
{
	char packet[PACKET_SIZE] = { 0 };
	header hdr = { 0 };

	hdr.command = GET_LIST;
	memcpy(packet, &hdr, sizeof(header));

	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(client_node)) != 0)
		{
			memcpy(&packet[sizeof(header)], &routing_table[node_idx], sizeof(client_node));
			send(sock, packet, PACKET_SIZE, 0);
		}
	}

}

static void show_node_data(client_node * client)
{
	struct in_addr addr;
	addr.s_addr = client->ip;

	printf("%s\t %d\t %d\t\n", inet_ntoa(addr), client->port, client->time);
}

void send_packet_udp(client_node * node, int command, void * data, size_t size_of_data)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in dest_addr;
	char packet[PACKET_SIZE] = { 0 };

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = node->ip;
	dest_addr.sin_port = node->port;

	header *hdr = (header *) packet;
	hdr->command = command;

	memcpy(&packet[sizeof(header)], data, size_of_data);

	sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	closesocket(sock);
}

void send_packet_to_all_udp(int command, void * data, size_t size_of_data)
{
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(routing_table[node_idx])) != 0)
		{
			send_packet_udp(&routing_table[node_idx], command, data, size_of_data);
		}
	}
}

void send_iptable_to_node_udp(client_node * node)
{
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(routing_table[node_idx])) != 0)
		{
			client_node peer = routing_table[node_idx];
			peer.time = time(0) - peer.time;	/* send delta instead of registering time */

			send_packet_udp(node, ADD_NODE, &peer, sizeof(peer));
		}
	}
}

void show_iptable(void)
{
	system("cls");
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(client_node)) != 0)
			show_node_data(&routing_table[node_idx]);
	}
}
