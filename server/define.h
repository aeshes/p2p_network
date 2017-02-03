#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>

/* Element in routing table */
#pragma pack(push, 1)
typedef struct
{
	uint32_t ip;
	uint16_t port;
	uint32_t time;
} client_node;
#pragma pack(pop)


/* DONT FORGET: add size of data */
#pragma pack(push, 1)
typedef struct
{
	char command;
} header;
#pragma pack(pop)

/* Protocol properties */

#define PACKET_SIZE 1024

/* Command list */

#define ADD_NODE 	0x01
#define REM_NODE 	0x02
#define GET_LIST	0x03

#endif
