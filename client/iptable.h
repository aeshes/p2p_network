#ifndef IPTABLE_H
#define IPTABLE_H

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "proto.h"

void set_my_node(client_node * mynode);

/* Adds new node to the routing table */
void add_node(client_node * new_node);

void send_iptable(SOCKET sock);

void update_iptable(void);

void send_packet_udp(client_node * node, int command, void * data, size_t size_of_data);

void send_packet_to_all_udp(int command, void * data, size_t size_of_data);

void send_iptable_to_node_udp(client_node * node);

void show_iptable(void);


#endif
