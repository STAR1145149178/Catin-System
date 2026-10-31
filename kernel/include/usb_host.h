#ifndef _USB_HOST_H
#define _USB_HOST_H

#include "types.h"

// USB 控制传输接口（用于配置设备、读寄存器等）
int usb_control_transfer(u8 dev_addr, u8 bmRequestType, u8 bRequest, u16 wValue, u16 wIndex, void *data, u16 wLength);

// USB 批量传输接口（用于网卡收发以太网帧）
int usb_bulk_transfer(u8 ep, void *data, int len, int is_in);

// 初始化 USB 主机控制器
void usb_host_init(void);

#endif