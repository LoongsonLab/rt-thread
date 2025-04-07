**QEMU/LoongArch64 VIRT 板级支持包使用说明**


<!-- TOC -->

- [1. 简介](#1-简介)
- [2. 构建](#2-构建)
	- [2.1. 安装工具链](#21-安装工具链)
	- [2.2. 设置 RT-Thread 工具链环境变量](#22-设置-rt-thread-工具链环境变量)
	- [2.3. 下载内核](#23-下载内核)
	- [2.4. 配置内核](#24-配置内核)
	- [2.5. 编译内核](#25-编译内核)
- [3. 运行](#3-运行)
	- [3.2. 运行 QEMU](#32-运行-qemu)
		- [3.2.1. 运行 RT-Thread 标准版](#321-运行-rt-thread-标准版)

<!-- /TOC -->


# 1. 简介

LoongArch（龙架构）是龙芯中科自主研发的一种新的RISC指令集架构，旨在提高计算机的性能和效率。LoongArch于2020年推出，基于二十年的CPU研制和生态建设积累，具有自主性、先进性与兼容性。

本工程使用的Qemu，模拟的是3A5000 develop board emulation。

# 2. 构建

工作系统：以 Ubuntu 24.04 为例：

```shell
$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 24.04.2 LTS
Release:	24.04
Codename:	noble
```

## 2.1. 安装工具链

### 2.1.1 安装x86_64交叉编译器工具链

下载LoongArch64的GCC交叉编译器, 在[这里](https://github.com/LoongsonLab/oscomp-toolchains-for-oskernel/releases/)。
``` shell
$ wget https://github.com/LoongsonLab/oscomp-toolchains-for-oskernel/releases/download/loongarch64-cross-toolchains-qemu/loongarch64-musl-gcc-nightly-2025-3-27.tar.gz

# 解压到/opt下
$ tar zxf loongarch64-musl-gcc-nightly-2025-3-27.tar.gz

# 将/opt/loongarch64/bin添加到系统环境变量
$ PATH=${PATH}:/opt/loongarch64/bin

# 测试gcc，输入如下
$ loongarch64-linux-musl-gcc -v

Using built-in specs.
COLLECT_GCC=./loongarch64-linux-musl-gcc
COLLECT_LTO_WRAPPER=/home/airxs/user/oscomp/rtthread/loong64/loongarch64/bin/../libexec/gcc/loongarch64-linux-musl/14.2.0/lto-wrapper
Target: loongarch64-linux-musl
Configured with: /home/airxs/user/github/toolchains/loongarch-gnu-toolchain/gcc/configure --target=loongarch64-linux-musl --prefix=/home/airxs/user/github/toolchains/loongarch-gnu-toolchain/loongarch64-lp64d-linux --with-sysroot=/home/airxs/user/github/toolchains/loongarch-gnu-toolchain/loongarch64-lp64d-linux/sysroot --with-system-zlib --enable-shared --enable-tls --enable-languages=c,c++ --disable-libmudflap --disable-libssp --disable-libquadmath --disable-libsanitizer --disable-nls --with-simd=none --disable-bootstrap --src=.././gcc --enable-default-pie --disable-multilib --with-abi=lp64d --with-arch=loongarch64 --with-tune=generic 'CFLAGS_FOR_TARGET=-O2    -mcmodel=normal' 'CXXFLAGS_FOR_TARGET=-O2    -mcmodel=normal'
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 14.2.0 (GCC)

```

### 2.1.2 安装LoongArch 本地(Native)编译器工具链

下载LoongArch64本地的Musl库以及相关脚本, 在[这里](https://github.com/LoongsonLab/oscomp-toolchains-for-oskernel/releases/)。
``` shell
$ wget https://github.com/LoongsonLab/oscomp-toolchains-for-oskernel/releases/download/loongarch64-musl-native/musl-1.2.5.tar.gz

# 解压到/opt下
$ tar zxf musl-1.2.5.tar.gz

# 设置和更新Musl库路径，可以查看musl-1.2.5下的README.md
$ cd musl-1.2.5 && ./setup

# 将/opt/musl-1.2.5/bin添加到系统环境变量
$ PATH=${PATH}:/opt/musl-1.2.5/bin

# 测试gcc
$ musl-gcc -v

```

## 2.2. 设置 RT-Thread 工具链环境变量

RT-Thread 工具链相关的环境变量，根据上面解压的路径。
```shell
$ export RTT_EXEC_PATH=/opt/loongarch64/bin
```
如果是LoongArch本地编译，设置如下：
```shell
$ export RTT_CC_PREFIX=musl-
```

## 2.3. 下载内核

假设工作路径是 `$WORKSPACE`。

```shell
$ cd $WORKSPACE
$ git clone git@github.com:RT-Thread/rt-thread.git
```

进入 qemu-virt64-loongarch 所在 BSP 目录，后面的操作不做另外介绍，默认就在这个目录下。

```shell
$ cd $WORKSPACE/rt-thread/bsp/qemu-virt64-loongarch
```

## 2.4. 配置内核

第一次编译前先刷新一下配置文件。

```shell
$ scons --menuconfig
```

默认配置就是 RT-Thread 标准版，所以如果没有什么特别需求，什么都不要改动，直接保存退出即可。

## 2.5. 编译内核

如果以前编译后，可以清理一下：

```shell
$ scons --clean
```

或者直接编译：

```shell
$ scons -j$(nproc)
```

在 `$WORKSPACE/rt-thread/bsp/qemu-virt64-loongarch` 路径下会生成内核的二进制文件 `rtthread.elf 和 rtthread.bin`。

# 3. 运行

## 3.1. 安装 QEMU

```shell
# QEMU我们已经在上面的工具链中，在bin文件夹下，按照上面配置好的环境变量，直接使用。
```

安装完毕后可以看一下版本。

```shell
$ qemu-system-loongarch64 --version
QEMU emulator version 9.2.2
Copyright (c) 2003-2024 Fabrice Bellard and the QEMU Project developers
```

## 3.2. 运行 QEMU

仓库里已经提供了现成的执行脚本，可以直接执行：

```shell
$ ./run.sh
```


### 3.2.1. 运行 RT-Thread 标准版

示例如下：

```shell
$ ./run.sh

./run.sh
heap: [0x00273e48 - 0x04273e48]

 \ | /
- RT -     Thread Operating System
 / | \     5.2.0 build Mar 15 2025 17:37:02
 2006 - 2024 Copyright by RT-Thread team
Hello LoongArch64!
msh />ps
thread   pri  status      sp     stack size max used left tick   error  tcb addr
-------- ---  ------- ---------- ----------  ------  ---------- ------- ------------------
tshell    20  running 0x00000438 0x00001000    49%   0x00000007 OK      0x9000000000e76278
mmcsd_de  22  suspend 0x00000438 0x00000400    100%   0x00000014 EINTRPT 0x9000000000270d20
tidle0    31  ready   0x000004c0 0x00001000    41%   0x00000018 OK      0x90000000002713e8
timer      4  suspend 0x000003f8 0x00001000    36%   0x00000009 EINTRPT 0x90000000002728d8
msh />
msh />ls
Directory /:
mnt                 <DIR>                    
loongarch64         <DIR>                    
loongsonlab         <DIR> 
msh />
```

