#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// 禁掉标准库头文件
#define LWIP_NO_CTYPE_H 1
#define LWIP_NO_INTTYPES_H 1
#define LWIP_NO_STDINT_H 1
#define LWIP_NO_STDDEF_H 1
#define LWIP_NO_LIMITS_H 1     // 新增：禁掉 limits.h
#define LWIP_NO_STRING_H 1     // 新增：提前禁掉 string.h

// ... 下面保留你之前的那些宏定义（NO_SYS, MEM_SIZE等）不变 ...

#define NO_SYS 1
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE (1024 * 128)
#define MEMP_NUM_PBUF 16
#define MEMP_NUM_TCP_PCB 8
#define PBUF_POOL_SIZE 16
#define LWIP_ARP 1
#define LWIP_ETHERNET 1
#define LWIP_IPV4 1
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_ICMP 1
#define LWIP_DHCP 1
#define LWIP_DNS 1
#define TCP_MSS 1460
#define LWIP_NETIF_API 0
#define LWIP_COMPAT_MUTEX 1

#endif