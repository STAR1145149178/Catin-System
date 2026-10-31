#!/bin/bash
# ============================================================
#  Catin Kernel 2.2 — 全自动双栖构建脚本 (最终版)
# ============================================================

if [[ "$PREFIX" == *"com.termux"* ]]; then
    echo "=================================================="
    echo " [*] 检测到原生 Termux 环境，准备进入 Ubuntu 容器..."
    echo "=================================================="
    if [ ! -d "$PREFIX/var/lib/proot-distro/installed-rootfs/ubuntu" ]; then
        echo " [✗] 找不到 Ubuntu 容器，请先手动安装！"
        exit 1
    fi
    proot-distro login ubuntu --bind /storage/emulated/0:/host -- bash /host/Catin1/build_all.sh || true
    echo "=================================================="
    echo " 🎉 构建流程结束，已退出容器，返回 Termux！"
    echo "=================================================="
    exit 0
fi

echo "=================================================="
echo " [*] 已进入 Ubuntu 容器，开始构建 Catin OS..."
echo "=================================================="

cd /host/Catin1 || { echo "✗ 找不到 /host/Catin1 目录"; exit 1; }

CROSS="aarch64-linux-gnu-"
CFLAGS="-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-pic -fno-pie -Wall -Wextra -O2 -fno-common -fno-stack-protector -Ikernel/include -Ikernel/net -Ithird_party/lwip/src/include -Ithird_party/lwip/src/include/ipv4"

COMMON_C=(
    kernel/core/main.c kernel/core/exec.c
    kernel/lib/string.c kernel/lib/printf.c kernel/lib/mem.c kernel/lib/font8x16.c
    kernel/lib/log.c kernel/system/boot_cfg.c kernel/system/boot_loader.c
    kernel/security/security.c kernel/mm/mm.c kernel/core/arch_common.c
    kernel/fs/vfs.c kernel/proc/proc.c kernel/proc/syscall.c
    kernel/ui/gesture.c kernel/ui/xcui_core.c kernel/ui/xcui_draw.c
    kernel/ui/xcui_text.c kernel/ui/xcui_input.c kernel/ui/xcui_widget.c kernel/ui/xcui_toast.c
    kernel/apps/demo.c kernel/apps/panels.c kernel/apps/shell.c
    kernel/apps/codec_test.c kernel/apps/wallpaper.c
    kernel/codec/codec_common.c kernel/codec/wav.c kernel/codec/mp3.c kernel/codec/mp4.c
    kernel/codec/png.c kernel/codec/jpg.c kernel/codec/gif.c
    kernel/core/arm_common.c kernel/core/fdt.c
    kernel/drivers/fb.c kernel/drivers/virtio_mmio.c
    kernel/drivers/virtio_input.c kernel/drivers/input.c
    kernel/drivers/usb_host.c kernel/drivers/rtl8152.c
    kernel/net/ethernetif.c kernel/net/sys_arch.c
    
    # lwIP 核心源码
    third_party/lwip/src/core/init.c
    third_party/lwip/src/core/def.c
    third_party/lwip/src/core/inet_chksum.c
    third_party/lwip/src/core/ip.c
    third_party/lwip/src/core/mem.c
    third_party/lwip/src/core/memp.c
    third_party/lwip/src/core/netif.c
    third_party/lwip/src/core/pbuf.c
    third_party/lwip/src/core/stats.c
    third_party/lwip/src/core/sys.c
    third_party/lwip/src/core/tcp.c
    third_party/lwip/src/core/tcp_in.c
    third_party/lwip/src/core/tcp_out.c
    third_party/lwip/src/core/timeouts.c
    third_party/lwip/src/core/udp.c
    third_party/lwip/src/core/dns.c
    
    # lwIP IPv4 专用源码
    third_party/lwip/src/core/ipv4/ip4.c
    third_party/lwip/src/core/ipv4/ip4_addr.c
    third_party/lwip/src/core/ipv4/ip4_frag.c
    third_party/lwip/src/core/ipv4/icmp.c
    third_party/lwip/src/core/ipv4/etharp.c
    third_party/lwip/src/core/ipv4/dhcp.c
    third_party/lwip/src/core/ipv4/acd.c
    
    # lwIP 网卡底层处理
    third_party/lwip/src/netif/ethernet.c
)
ASM=( arch/arm64/boot.S arch/arm64/mmu.S arch/arm64/vectors.S arch/arm64/context.S )

line() { printf '%.0s─' $(seq 1 60); echo; }

line
echo "  [AArch64] 编译内核中..."
line

outdir="out/a64"
rm -rf "$outdir"; mkdir -p "$outdir"
objs=()

for src in "${ASM[@]}"; do
    obj="$outdir/$(echo ${src%.S} | tr '/' '_').o"
    "${CROSS}gcc" $CFLAGS -c "$src" -o "$obj" || exit 1
    objs+=("$obj")
done

for src in "${COMMON_C[@]}"; do
    obj="$outdir/$(echo ${src%.c} | tr '/' '_').o"
    "${CROSS}gcc" $CFLAGS -c "$src" -o "$obj" || exit 1
    objs+=("$obj")
    printf "."
done
echo ""

elf="$outdir/kernel.elf"
"${CROSS}gcc" -nostdlib -no-pie -static -T arch/arm64/linker.ld -o "$elf" "${objs[@]}" -lgcc || exit 1

"${CROSS}objcopy" -O binary "$elf" kernel_a64.img
echo "  ✓ 模拟器内核生成: kernel_a64.img ($(stat -c%s kernel_a64.img) bytes)"

# ═══ 编译用户态应用 ═══
if [ -d user/lib ] && [ -d user/apps ]; then
    line
    echo "  编译用户态应用"
    line
    U_CFLAGS="-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-pic -fno-pie -Wall -O2 -Iuser/include"
    build_user_app() {
        local name=$1
        local dir="user/apps/$name"
        mkdir -p "$dir/bin"
        local u_objs=""
        for f in user/lib/crt0.S user/lib/syscall.c user/lib/string.c \
         user/lib/printf.c user/lib/malloc.c user/lib/fs.c \
         user/lib/gui.c user/lib/screen.c user/lib/net.c \
         user/lib/http.c user/lib/zip.c user/lib/zzc.c \
         user/lib/font8x16.c user/lib/platform.c \
         "$dir/main.c"; do
            [ -f "$f" ] || continue
            local o="/tmp/catin_$(basename $f).o"
            aarch64-linux-gnu-gcc $U_CFLAGS -c "$f" -o "$o" || exit 1
            u_objs="$u_objs $o"
        done
        aarch64-linux-gnu-ld -static -no-pie -Ttext=0x200000 --entry=_start -o "$dir/bin/app" $u_objs || exit 1
        echo "  ✓ $dir/bin/app"
        (cd "$dir" && zip -0 -qr "../$name.zzc" app.conf bin assets 2>/dev/null || true)
        [ -f "user/apps/$name.zzc" ] && echo "  ✓ user/apps/$name.zzc"
    }
    build_user_app settings
    build_user_app fm
    build_user_app browser
fi

# ═══ 将应用放入系统目录 ═══
line
echo "  把应用放进 Catin System"
line
mkdir -p "Catin System/rootfs/apps"
cp user/apps/*.zzc "Catin System/rootfs/apps/" 2>/dev/null || true

# ═══ 打包 QEMU 镜像 ═══
line
echo "  打包 catin.img (QEMU 模拟器专用)"
line
rm -f catin.img
dd if=/dev/zero of=catin.img bs=1M count=96 status=none
mkfs.vfat -F 32 -n CATIN catin.img >/dev/null
mcopy -i catin.img kernel_a64.img "::/kernel_a64.img"
echo "  ✓ catin.img 生成成功 ($(stat -c%s catin.img) bytes)"

# ═══ 打包真机刷机包 ═══
line
echo "  打包真机 boot.img"
line

if [ -d "Catin System" ]; then
    echo "  [*] 自动处理 A/B 分区备份..."
    mkdir -p "Catin System/OS"
    cp kernel_a64.img "Catin System/OS/boot.img"
    cp kernel_a64.img "Catin System/OS/boot.b.img"
    echo "  ✓ 已注入: Catin System/OS/boot.img"
    echo "  ✓ 已注入: Catin System/OS/boot.b.img"

    echo "  [*] 打包 Catin System 为 ramdisk..."
    (cd "Catin System" && find . | cpio -o -H newc 2>/dev/null | gzip > ../ramdisk.gz)
    
    if [ -f "ramdisk.gz" ]; then
        echo "  [*] 合成 boot_r5.img..."
        if command -v mkbootimg >/dev/null 2>&1; then
            mkbootimg \
                --kernel kernel_a64.img --ramdisk ramdisk.gz \
                --base 0x80000000 --kernel_offset 0x00000000 \
                --ramdisk_offset 0x01000000 --tags_offset 0x00000100 \
                --pagesize 2048 --cmdline "console=ttyMSM0,115200n8 earlycon" \
                --output boot_r5.img 2>/dev/null || true
        fi
        if [ ! -f "boot_r5.img" ] && [ -f "mkbooting.py" ]; then
            echo "  [!] 使用 mkbooting.py 打包..."
            python3 mkbooting.py --kernel kernel_a64.img --ramdisk ramdisk.gz \
                --cmdline "console=ttyMSM0,115200n8 earlycon" --output boot_r5.img
        fi
        
        if [ -f "boot_r5.img" ]; then
            echo "  ✓ boot_r5.img (真机刷机包)"
        else
            echo "  ✗ 打包失败"
        fi
        rm -f ramdisk.gz
    fi
fi

line
echo "  构建完成！"
echo "    真机刷🐓: fastboot boot boot_r5.img"
echo "    模拟器跑: qemu-system-aarch64 -M virt -cpu cortex-a53 -m 512M -kernel kernel_a64.img -device ramfb -device virtio-tablet-device -serial mon:stdio"
line