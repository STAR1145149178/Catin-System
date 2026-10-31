#ifndef _CATIN_NET_H
#define _CATIN_NET_H
#include "catin_types.h"
#define CATIN_SOCK_TCP 1
#define CATIN_SOCK_UDP 2
int catin_net_socket(int type);
int catin_net_connect(int s, u32 ip_be, u16 port);
int catin_net_send(int s, const void *buf, int n);
int catin_net_recv(int s, void *buf, int n);
int catin_net_close(int s);
u32 catin_net_dns(const char *hostname);
u32 catin_net_parse_ip(const char *ip_str);
#endif
