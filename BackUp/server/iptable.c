#include "iptable.h"

#include <stdlib.h>
#include <string.h>


#define NODE_TABLE_SIZE 255

static const client_node null_node = { 0 };
static client_node routing_table[NODE_TABLE_SIZE] = { 0 };
static size_t node_ptr = 0;
static size_t node_count = 0;


static client_node * node_already_exists(client_node * node)
{
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (routing_table[node_idx].ip == node->ip)
			return &routing_table[node_idx];
	}
	return NULL;
}

static int compare_time_desc(const void *x, const void *y)
{
	return ((client_node *)y)->time - ((client_node *)x)->time;
}

static void sort_by_time_desc(void)
{
	qsort(routing_table, node_count, sizeof(client_node), compare_time_desc);
}

void add_node(client_node * new_node)
{
	client_node *tmp = NULL;

	//if ((tmp = node_already_exists(new_node)) != NULL)
	//{
		/* Update time */
	//	tmp->time = new_node->time;
	//	tmp->port = new_node->port;
	//}
	//else
	{
		routing_table[node_ptr++] = *new_node;
		node_ptr %= NODE_TABLE_SIZE;
		if (node_count < NODE_TABLE_SIZE)
			++node_count;
	}
	sort_by_time_desc();
}

void send_iptable(SOCKET sock)
{
	char packet[PACKET_SIZE] = { 0 };
	header hdr = { 0 };

	hdr.command = GET_LIST;
	hdr.size_of_data = sizeof(routing_table[0]);
	memcpy(packet, &hdr, sizeof(header));

	printf("Sending node list: %d nodes\n", node_count);

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

void show_iptable(void)
{
	printf("IP\t PORT\t TIME\n");
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(client_node)) != 0)
			show_node_data(&routing_table[node_idx]);
	}
}
