# Catin-System
# Catin OS (Catin1)
Since this code is open source, any modified version you release must also be released as open source.

联系方式 / Contact Information

作者信息 / Author Information: 中国人 / Chinese

邮箱 / Email:

· 3757835685@qq.com
· qq3757835685@163.com
· qq3757835685@126.com

QQ: 3757835685

> **An ambitious microkernel-based operating system, entirely written from scratch on a mobile phone using Termux.**

![Architecture](https://img.shields.io/badge/Architecture-AArch64%20%7C%20x86-blue)
![Language](https://img.shields.io/badge/Language-C%20%7C%20Assembly-orange)
![Environment](https://img.shields.io/badge/Developed%20On-Mobile%20%2F%20Termux-success)
![License](https://img.shields.io/badge/License-MIT-green)

## 📖 Introduction

**Catin OS** is not just a toy project; it is a fully functional, bare-metal operating system ecosystem. What makes this project truly unique is the background of its creator: **a 13-year-old developer who wrote the entire kernel, drivers, UI framework, and application ecosystem purely on a mobile phone using Termux, without access to a traditional PC.**

From paging and scheduling to multimedia decoding and a custom application format (`.zzc`), Catin OS pushes the absolute limits of mobile-based development.

## ✨ Key Features

- **🛠️ Advanced Microkernel Architecture:** Written in pure C and Assembly. Features custom memory management, process scheduling, and system calls.
- **📱 Custom Native UI Framework (XCUI):** A completely hand-written 2D graphics library. Features a Launcher, Status Bar, Control Center, Soft Keyboard, and even an embedded C-array wallpaper.
- **🎵 From-Scratch Multimedia Codecs:** Zero external dependencies. Implements WAV, MP3, MP4, PNG, JPG, and GIF decoding. Features hand-written DEFLATE (inflate) and IDCT (Taylor series approximation) algorithms.
- **🌐 Real Network Stack:** Ported **lwIP** to the bare-metal environment. Coupled with a custom USB Host (DWC3) and RTL8152 Ethernet driver, allowing the OS to access the internet via a wired USB network card.
- **📦 Custom App Ecosystem (`.zzc`):** Features a custom executable format based on ZIP. Includes an ELF loader, custom syscall ABIs, and a native App Store style package manager.
- **🛡️ Robust Security & A/B Partitions:** Simulates Android's Bootloader (BL) lock mechanism. Features anti-brick A/B dual-boot partition logic (`boot_status.cfg`), crash-counting, and recovery fallback.
- **🤖 AI Native (Agent):** Includes a local Python AI Agent (`Agnet.py`) running on Termux, capable of interacting with local GGUF/PTH models to assist with coding and system operations.
- **🚀 True Hardware Boot:** Can be compiled and packed into a standard Android `boot.img` (Header v0) using a custom `mkbooting.py` script, and flashed onto real devices (tested on OPPO R5 / Snapdragon MSM8939).

## 🏗️ Technical Architecture

- **Boot Flow:** Custom U-Boot/DWC3 logic. Parses Device Tree Blobs (DTB). Hardware Abstraction Layer (HAL) for ARMv8-A (AArch64) and x86 (partial support).
- **File System:** Custom Virtual File System (VFS) with registry-driven app management.
- **Build System:** Fully automated cross-compilation pipeline. Uses `proot-distro` to run an Ubuntu container inside Termux, compiling the kernel via `aarch64-linux-gnu-gcc`.

## 🚀 Quick Start

### 1. Compile the Kernel
Because the developer writes code on a phone, the build script is designed for Termux + Proot Ubuntu:
# Inside Termux
git clone https://github.com/YOUR_USERNAME/Catin-OS.git
cd Catin-OS
chmod +x build_all.sh
./build_all.sh

2. Run in QEMU

qemu-system-aarch64 -M virt -cpu cortex-a53 -m 512M -kernel kernel_a64.img -device ramfb -device virtio-tablet-device -serial mon:stdio

3. Flash to Real Device (e.g., OPPO R5)

The build script will generate a boot_r5.img. You can test it temporarily via Fastboot:


fastboot boot boot_r5.img

(Warning: Flashing custom kernels to real hardware carries risks. Ensure you have a way to recover your device.)

⚖️ License & Third-Party

This project's core code is licensed under the MIT License.
This project uses lwIP for networking. All third-party license declarations can be found in THIRD_PARTY_LICENSES.md.

Code have some ERROR is AI Fix

---

Catin OS (Catin1) - 中文说明

一个纯手机（Termux）手搓的、基于微内核架构的野心勃勃的操作系统。

这个代码是一个开源代码，如果要是更改后续发布，必须开源发布。

📖 项目简介

Catin OS 绝不是一个简单的玩具项目，而是一个五脏俱全的裸机操作系统生态。这个项目最独特的地方在于它的开发背景：作者是一名13岁的初中生，在没有电脑的情况下，完全使用手机在 Termux 终端中，纯手写了内核、驱动、UI框架以及应用生态。

从内存分页、进程调度，到多媒体解码、自定义应用格式（.zzc），Catin OS 挑战了移动端开发的极限。

✨ 核心特性

· 🛠️ 进阶微内核架构：纯 C 语言与汇编编写，拥有独立的内存管理、进程调度与系统调用机制。
· 📱 自研原生 UI 框架 (XCUI)：完全手写的 2D 图形库。包含桌面启动器、状态栏、控制中心、软键盘，甚至内置了一张 C 语言数组壁纸。
· 🎵 零依赖多媒体解码器：从零实现了 WAV, MP3, MP4, PNG, JPG, GIF 的编解码。其中包含手写的 DEFLATE 解压算法与泰勒级数逼近的 IDCT 算法。
· 🌐 真实可用的网络栈：将 lwIP 协议栈移植到了裸机。配合自研的 USB Host (DWC3) 与 RTL8152 网卡驱动，让系统能够通过外接有线网卡上网。
· 📦 自研应用生态 (.zzc)：基于 ZIP 的自定义可执行文件格式，包含 ELF 加载器、自定义系统调用（Syscall）规范。
· 🛡️ 极强的安全与防变砖机制：模拟了安卓的 Bootloader (BL) 锁机制。拥有 A/B 双分区防变砖逻辑（boot_status.cfg）、崩溃计数与自动切换恢复机制。
· 🤖 AI 原生融合：附带了运行在 Termux 的本地 Python AI Agent (Agnet.py)，能对接本地 GGUF/PTH 模型，辅助编程与系统操作。
· 🚀 真机刷机验证：利用手写的 mkbooting.py 脚本，可将内核打包为标准安卓 boot.img (Header v0)，并成功在真实设备上引导（已在 OPPO R5 / 骁龙 MSM8939 上验证）。

🏗️ 技术架构

· 引导流程：硬核的 U-Boot/DWC3 底层逻辑，支持解析设备树（DTB），针对 ARMv8-A (AArch64) 架构提供了完整的 HAL 层支持。
· 文件系统：自研的虚拟文件系统（VFS），基于注册表的应用管理机制。
· 构建系统：全自动交叉编译流水线。在 Termux 中使用 proot-distro 切入 Ubuntu 容器，通过 aarch64-linux-gnu-gcc 编译内核。

🚀 快速开始

1. 编译内核

由于作者是在手机上开发，构建脚本专为 Termux + Proot Ubuntu 设计：

# 在 Termux 中执行
git clone https://github.com/YOUR_USERNAME/Catin-OS.git
cd Catin-OS
chmod +x build_all.sh
./build_all.sh

2. 在 QEMU 模拟器运行

qemu-system-aarch64 -M virt -cpu cortex-a53 -m 512M -kernel kernel_a64.img -device ramfb -device virtio-tablet-device -serial mon:stdio


3. 真机测试 (如 OPPO R5)

构建脚本会生成 boot_r5.img。你可以通过 Fastboot 临时启动测试（不写入存储）：

fastboot boot boot_r5.img

(警告：向真实硬件刷入自定义内核存在风险，请确保你有救砖手段。)

⚖️ 开源协议与第三方声明

本项目的核心代码采用 MIT License 开源。
本项目在网络部分使用了 lwIP 协议栈。所有的第三方开源协议声明均可在 THIRD_PARTY_LICENSES.md 文件中查看。

代码中有部分报错，由ai修改
本人英语不怎么好
这个文件是 AI 给我生成的
不出意外的话，我的主页应该也有我开源的内核。大家也可以改这个内核
