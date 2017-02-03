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

	if ((tmp = node_already_exists(new_node)) != NULL)
	{
		/* Update time */
		tmp->time = new_node->time;
	}
	else
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
	printf("---- Node info ----\n");
	printf("IP : %d\n", 	client->ip);
	printf("PORT : %d\n", 	client->port);
	printf("TIME : %d\n", 	client->time);

	printf("RAW DATA: ");
	for (int i = 0; i < sizeof(client_node); ++i)
		printf("0x%02x ", ((unsigned char *)client)[i]);

	printf("\n\n");
}

void show_iptable(void)
{
	for (size_t node_idx = 0; node_idx < NODE_TABLE_SIZE; ++node_idx)
	{
		if (memcmp(&routing_table[node_idx], &null_node, sizeof(client_node)) != 0)
			show_node_data(&routing_table[node_idx]);
	}
}
