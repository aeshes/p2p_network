#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <stdint.h>
#include "proto.h"

//	Connects to server which hash a table of recent peers
SOCKET connect_to_server(char *host, uint16_t port);

//	Once p2p-client runs, it registers in the peer table of server
void register_on_server(SOCKET server, client_node *mynode);

// Sends fake registering request. Need for tests.
void send_fake_nodes(SOCKET sock);

// This function loads a list of peers from server into client table
void get_nodes_from_server(SOCKET server_socket);

#endif
