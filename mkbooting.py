#!/usr/bin/env python3
import struct, hashlib, sys, os

# Android Boot Image Header v0 常量
BOOT_MAGIC = b"ANDROID!"
BOOT_NAME_SIZE = 16
BOOT_ARGS_SIZE = 512
BOOT_EXTRA_ARGS_SIZE = 1024
PAGE_SIZE = 2048

def pad_file(f, page_size):
    """将文件填充到页对齐"""
    f.seek(0, 2)  # 跳到文件末尾
    pos = f.tell()
    padding = (page_size - (pos % page_size)) % page_size
    if padding:
        f.write(b'\x00' * padding)

def main():
    # 1. 解析参数（只处理我们需要的）
    args = {}
    for i in range(1, len(sys.argv), 2):
        key = sys.argv[i].lstrip('-')
        if i + 1 < len(sys.argv):
            args[key] = sys.argv[i+1]

    kernel_path = args.get('kernel')
    ramdisk_path = args.get('ramdisk')
    output_path = args.get('output', 'boot.img')
    cmdline_str = args.get('cmdline', '').encode()[:BOOT_ARGS_SIZE-1]
    # 这里我们忽略 base 等参数，因为它们主要影响内核的加载地址，而我们的内核是链接在固定地址的

    if not kernel_path or not ramdisk_path:
        print("错误: 需要 --kernel 和 --ramdisk 参数")
        sys.exit(1)

    print(f"[*] 读取内核: {kernel_path}")
    with open(kernel_path, 'rb') as f:
        kernel_data = f.read()

    print(f"[*] 读取内存盘: {ramdisk_path}")
    with open(ramdisk_path, 'rb') as f:
        ramdisk_data = f.read()

    # 2. 计算SHA1校验和（用于头部）
    sha = hashlib.sha1()
    sha.update(kernel_data)
    sha.update(struct.pack('<I', len(kernel_data)))
    sha.update(ramdisk_data)
    sha.update(struct.pack('<I', len(ramdisk_data)))
    digest = sha.digest()

    # 3. 构建Boot Image Header v0
    print("[*] 构建头部...")
    # 头部结构: magic, kernel_size, kernel_addr, ramdisk_size, ramdisk_addr, second_size, second_addr, tags_addr, page_size, header_version, os_version, name, cmdline, id, extra_cmdline
    header = struct.pack(
        '<8s10I16s512s32s1024s',
        BOOT_MAGIC,
        len(kernel_data),
        0x80008000,  # kernel_addr (默认值，对 header v0 通常不重要)
        len(ramdisk_data),
        0x81000000,  # ramdisk_addr (默认值)
        0,           # second_size
        0,           # second_addr
        0x80000100,  # tags_addr
        PAGE_SIZE,
        0,           # header_version (0 = 传统)
        0,           # os_version
        b'CatinOS',  # name
        cmdline_str, # cmdline
        digest,      # id (SHA1)
        b''          # extra_cmdline
    )

    # 4. 写入文件
    print(f"[*] 写入输出文件: {output_path}")
    with open(output_path, 'wb') as f:
        f.write(header)
        # 头部本身需要页对齐
        pad_file(f, PAGE_SIZE)
        f.write(kernel_data)
        pad_file(f, PAGE_SIZE)
        f.write(ramdisk_data)
        pad_file(f, PAGE_SIZE)

    print("✅ boot.img 打包成功！")

if __name__ == '__main__':
    main()