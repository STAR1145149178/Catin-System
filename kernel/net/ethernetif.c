#include "catin.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/etharp.h"
#include "lwip/def.h"

extern int rtl8152_send(const void *buf, int len);
extern int rtl8152_recv(void *buf, int max_len);

err_t low_level_output(struct netif *netif, struct pbuf *p) {
    (void)netif; // 消除警告
    struct pbuf *q;
    u8_t buffer[1500];
    int len = 0;
    
    for(q = p; q != NULL; q = q->next) {
        kmemcpy(buffer + len, q->payload, q->len);
        len += q->len;
    }
    rtl8152_send(buffer, len);
    return ERR_OK;
}

void ethernetif_input(struct netif *netif) {
    struct pbuf *p;
    u8_t buffer[1500];
    int len = rtl8152_recv(buffer, sizeof(buffer));
    
    if (len > 0) {
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != NULL) {
            pbuf_take(p, buffer, len);
            if (netif->input(p, netif) != ERR_OK) {
                pbuf_free(p);
            }
        }
    }
}

err_t ethernetif_init(struct netif *netif) {
    netif->name[0] = 'e';
    netif->name[1] = '0';
    
    netif->linkoutput = low_level_output;
    netif->output = etharp_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    return ERR_OK;
}