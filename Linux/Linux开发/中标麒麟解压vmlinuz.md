**这样做的原因主要是中标麒麟不能调试内核**


## vmlinux,vmlinuz,bzImage,zImage

### vmlinux

vmlinux是未压缩的内核，是make工作编译出的原始内核，vmlinuz是vmlinux的压缩文件。

vmlinux 是ELF文件，即编译出来的最原始的文件。

### zImage, bzImage和vmlinuz

vmlinuz是可引导的、压缩的内核。“vm”代表“Virtual Memory”。Linux 支持虚拟内存，不像老的操作系统比如DOS有640KB内存的限制。Linux能够使用硬盘空间作为虚拟内存，因此得名“vm”。vmlinuz是可执行的Linux内核，它位于/boot/vmlinuz，它一般是一个软链接,是bzImage/zImage文件的拷贝或指向bzImage/zImage的链接。

vmlinuz的建立有两种方式。

- 一是编译内核时通过“make zImage”创建，然后通过：“cp /usr/src/linux-2.4/arch/i386/linux/boot/zImage /boot/vmlinuz”产生。zImage适用于小内核的情况，它的存在是为了向后的兼容性。

- 二是内核编译时通过命令make bzImage创建，然后通过：“cp /usr/src/linux-2.4/arch/i386/linux/boot/bzImage /boot/vmlinuz”产生。bzImage是压缩的内核映像，需要注意，bzImage不是用bzip2压缩的，bzImage中的bz容易引起误解，bz表示“big zImage”。 bzImage中的b是“big”意思。

zImage(vmlinuz)和bzImage(vmlinuz)都是用gzip压缩的。它们不仅是一个压缩文件，而且在这两个文件的开头部分内嵌有gzip解压缩代码。所以你不能用gunzip 或 gzip –dc解包vmlinuz。

内核文件中包含一个微型的gzip用于解压缩内核并引导它。两者的不同之处在于，老的zImage解压缩内核到低端内存(第一个640K)，bzImage解压缩内核到高端内存(1M以上)。如果内核比较小，那么可以采用zImage 或bzImage之一，两种方式引导的系统运行时是相同的。大的内核采用bzImage，不能采用zImage。

> 但是注意通常情况下是不能用vmlinuz解压缩得到vmlinux的


## 解压vmlinuz

```
[root@localhost boot]# file vmlinuz-2.6.32-696.el6.x86_64 
vmlinuz-2.6.32-696.el6.x86_64: Linux kernel x86 boot executable bzImage, version 2.6.32-696.el6.x86_64 (mockbuil, RO-rootFS, root_dev 0x803, swap_dev 0x4, Normal VGA
[root@localhost boot]# od -t x1 -A d vmlinuz-2.6.32-696.el6.x86_64 | grep "1f 8b 08"
0014432 48 8d 83 b0 cf 40 00 ff e0 1f 8b 08 00 be 32 26
2934512 d8 7e 8c a6 3b 7a 1f 8b 08 1b 95 1b e2 1f b0 0b
[root@localhost boot]# dd if=vmlinuz-2.6.32-696.el6.x86_64 bs=1 skip=14441 | zcat > vmlinux

gzip: stdin: decompression OK, trailing garbage ignored

[root@localhost boot]# file vmlinux 
vmlinux: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped

```


参考：

http://smilejay.com/2013/06/extract-vmlinuz-and-initrd/

https://www.binss.me/blog/boot-process-of-linux-decompress-kernel/

https://www.cnblogs.com/alantu2018/p/8991298.html
