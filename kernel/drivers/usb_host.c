#include "catin.h"
#include "usb_host.h"

// OPPO R5 高通 MSM8939 的 DWC3 USB 控制器物理基址（大多数高通芯片都在这个地址）
#define USB_DWC3_BASE 0xF9200000ULL 

// 核心寄存器偏移量（从你发的 U-Boot core.c 中提取）
#define DWC3_GCTL         0xC110
#define DWC3_GUSB2PHYCFG0 0xC200
#define DWC3_GUSB3PIPECTL0 0xC2C0
#define DWC3_DCTL         0xC704

// GCTL 寄存器相关位
#define DWC3_GCTL_PRTCAPDIR_MASK (3 << 12)
#define DWC3_GCTL_PRTCAP_HOST    (1 << 12)
#define DWC3_GCTL_CORESOFTRESET  (1 << 30)

// GUSB2PHYCFG 寄存器相关位
#define DWC3_GUSB2PHYCFG_SUSPHY  (1 << 6)

// DCTL 寄存器相关位
#define DWC3_DCTL_CSFTRST        (1 << 30)

// 裸机简易延时函数 (用循环模拟)
static void usb_delay(int count) {
    for (volatile int i = 0; i < count * 1000; i++);
}

void usb_host_init(void) {
    log_info("[USB] DWC3 Host Controller Initializing...");
    u32 reg;

    // 1. 检查是否是 Synopsys DWC3 控制器 (可选，为了安全)
    // u32 snpsid = mmio_r32(USB_DWC3_BASE + 0xC100);
    // if ((snpsid & 0xFFFF0000) != 0x55330000) {
    //     log_err("[USB] Not a Synopsys DWC3 core!");
    //     return;
    // }

    // 2. 软复位 DWC3 核心
    reg = mmio_r32(USB_DWC3_BASE + DWC3_GCTL);
    reg |= DWC3_GCTL_CORESOFTRESET;
    mmio_w32(USB_DWC3_BASE + DWC3_GCTL, reg);
    
    // 等待复位完成（U-Boot 里是 read_poll_timeout，这里用死循环延时）
    usb_delay(100);
    while (mmio_r32(USB_DWC3_BASE + DWC3_GCTL) & DWC3_GCTL_CORESOFTRESET);

    // 3. 设置控制器为 Host 模式 (主机模式)
    reg = mmio_r32(USB_DWC3_BASE + DWC3_GCTL);
    reg &= ~DWC3_GCTL_PRTCAPDIR_MASK;
    reg |= DWC3_GCTL_PRTCAP_HOST;
    mmio_w32(USB_DWC3_BASE + DWC3_GCTL, reg);

    // 4. 配置 USB2.0 PHY (退出挂起状态)
    reg = mmio_r32(USB_DWC3_BASE + DWC3_GUSB2PHYCFG0);
    reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;
    mmio_w32(USB_DWC3_BASE + DWC3_GUSB2PHYCFG0, reg);
    
    usb_delay(100);

    log_info("[USB] Host Controller Initialized. Waiting for device...");
}

// 注意：以下两个函数需要实现完整的 TRB (Transfer Request Block) 机制
// 目前先用返回 -1 代表没收到硬件，等硬件到了我们继续完善。
int usb_control_transfer(u8 dev_addr, u8 bmRequestType, u8 bRequest, u16 wValue, u16 wIndex, void *data, u16 wLength) {
    (void)dev_addr; (void)bmRequestType; (void)bRequest; (void)wValue; (void)wIndex; (void)data; (void)wLength;
    // 真实发包逻辑待完善
    return -1; 
}

int usb_bulk_transfer(u8 ep, void *data, int len, int is_in) {
    (void)ep; (void)data; (void)len; (void)is_in;
    // 真实收发逻辑待完善
    return 0;
}