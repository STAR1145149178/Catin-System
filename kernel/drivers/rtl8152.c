#include "catin.h"
#include "usb_host.h"

#define RTL8152_REQ_GET_REGS 0x05
#define RTL8152_REQ_SET_REGS 0x05
#define RTL8152_RX_CTRL      0xE00A
#define RTL8152_TX_CTRL      0xE002
#define RTL8152_BMCR         0xE000 
#define RTL8152_MAAL0        0xE01A
#define RTL8152_MAAL1        0xE01C
#define RTL8152_MAAL2        0xE01E

static u8 ep_in, ep_out;
static u8 g_dev_addr;

static void rtl8152_write_reg(u16 reg, u16 value) {
    usb_control_transfer(g_dev_addr, 0x40, RTL8152_REQ_SET_REGS, reg, value, NULL, 0);
}

static u16 rtl8152_read_reg(u16 reg) {
    u16 value = 0;
    usb_control_transfer(g_dev_addr, 0xC0, RTL8152_REQ_GET_REGS, reg, 0, &value, 2);
    return value;
}

int rtl8152_init(u8 dev_addr, u8 in_ep, u8 out_ep) {
    g_dev_addr = dev_addr;
    ep_in = in_ep; 
    ep_out = out_ep;
    
    log_info("[RTL8152] Initializing Real Hardware...");

    rtl8152_write_reg(RTL8152_BMCR, 0x8000); 
    for(volatile int i=0; i<1000000; i++); 
    rtl8152_write_reg(RTL8152_BMCR, 0x2100); 

    rtl8152_write_reg(RTL8152_RX_CTRL, 0x000F); 
    rtl8152_write_reg(RTL8152_TX_CTRL, 0x0001); 
    
    log_info("[RTL8152] Hardware Ready.");
    return 0;
}

int rtl8152_send(const void *buf, int len) {
    if (len > 1500) return -1;
    u8 tx_buf[1600];
    tx_buf[0] = len & 0xFF;
    tx_buf[1] = (len >> 8) & 0xFF;
    tx_buf[2] = 0; tx_buf[3] = 0;
    tx_buf[4] = 0; tx_buf[5] = 0; tx_buf[6] = 0; tx_buf[7] = 0; 
    kmemcpy(tx_buf + 8, buf, len);
    return usb_bulk_transfer(ep_out, tx_buf, len + 8, 0);
}

int rtl8152_recv(void *buf, int max_len) {
    u8 rx_buf[2048];
    int total_len = usb_bulk_transfer(ep_in, rx_buf, sizeof(rx_buf), 1);
    if (total_len > 4) {
        int pkt_len = rx_buf[0] | (rx_buf[1] << 8); 
        if (pkt_len > 1500 || pkt_len == 0) return 0;
        if (pkt_len > max_len) pkt_len = max_len;
        kmemcpy(buf, rx_buf + 4, pkt_len);
        return pkt_len;
    }
    return 0;
}