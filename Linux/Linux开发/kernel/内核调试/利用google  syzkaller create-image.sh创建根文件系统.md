# 利用google / syzkaller create-image.sh创建根文件系统

## 下载create-image.sh

修改后的create-image.sh

[create-image.sh原始链接](https://raw.githubusercontent.com/google/syzkaller/master/tools/create-image.sh)



```sh
#!/bin/bash
# Copyright 2016 syzkaller project authors. All rights reserved.
# Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

# create-image.sh creates a minimal Debian Linux image suitable for syzkaller.

set -eux

# Create a minimal Debian distribution in a directory.
DIR=chroot
PREINSTALL_PKGS=openssh-server,curl,tar,gcc,libc6-dev,time,strace,sudo,less,psmisc,selinux-utils,policycoreutils,checkpolicy,selinux-policy-default,firmware-atheros

# If ADD_PACKAGE is not defined as an external environment variable, use our default packages
if [ -z ${ADD_PACKAGE+x} ]; then
# 修改下面这个位置，把想装的软件添加到下面
    ADD_PACKAGE="make,sysbench,git,vim,tmux,usbutils,tcpdump,ecryptfs-utils"
fi

```



修改下载源：

将地址修改为国内，方便快速下载

```shell
# 1. debootstrap stage

DEBOOTSTRAP_PARAMS="--include=$PREINSTALL_PKGS --components=main,contrib,non-free $RELEASE $DIR http://ftp.cn.debian.org/debian/"
if [ $ARCH != $(uname -m) ]; then
    DEBOOTSTRAP_PARAMS="--arch=$DEBARCH --foreign $DEBOOTSTRAP_PARAMS"
fi
sudo debootstrap $DEBOOTSTRAP_PARAMS
```



此脚本依赖`debootstrap`

```sh
apt-get install debootstrap
```





```sh
sh create-image.sh --feature full
```



## 加载内核启动

```sh
#! /bin/sh
qemu-system-x86_64 -kernel /root/bionic/arch/x86/boot/bzImage -drive file=/root/stretch.img -append "root=/dev/sda nokaslr" -m 4096 -s -S
```

