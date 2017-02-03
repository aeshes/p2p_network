#ifndef IPTABLE_H
#define IPTABLE_H

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "define.h"

/* Adds new node to the routing table */
void add_node(client_node * new_node);

void send_iptable(SOCKET sock);

void show_iptable(void);


#endif
