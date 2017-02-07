#ifndef IPTABLE_H
#define IPTABLE_H

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "proto.h"

/* Adds new node to the routing table */
void add_node(client_node * new_node);

void show_node_data(client_node * client);

void push_peer_list(SOCKET udp_sock, client_node * peer);

void pull_peer_list(SOCKET udp_sock, client_node * peer);

void broadcast_peer_list(SOCKET udp);

void show_iptable(void);


#endif
