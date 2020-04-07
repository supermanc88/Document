# Linux驱动调试

首先需要编译调试环境


## 编译调试版本的驱动

### 将想要调试的驱动加载到内存，查看内存中模块的地址：

```
[root@localhost dev]# cat /proc/modules | grep inline
input_event_inlinehook 2119 0 - Live 0xffffffffa058a000
[root@localhost dev]# 
```

### 查看驱动文件的区块信息

```
[root@localhost Documents]# objdump -h input_event_inlinehook.ko

input_event_inlinehook.ko:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .note.gnu.build-id 00000024  0000000000000000  0000000000000000  00000040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         00000174  0000000000000000  0000000000000000  00000070  2**4
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  2 .exit.text    00000031  0000000000000000  0000000000000000  000001e4  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  3 .init.text    00000057  0000000000000000  0000000000000000  00000215  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  4 .rodata.str1.1 00000071  0000000000000000  0000000000000000  0000026c  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  5 .modinfo      000000a1  0000000000000000  0000000000000000  000002e0  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
```

调试需要的是`.text`区块

### 添加调试符号

0xffffffffa058a000 + 0x00000070 = 0xffffffffa058a070

```
(gdb) add-symbol-file /home/superman/Documents/input_event_inlinehook.ko 0xffffffffa058a070
add symbol table from file "/home/superman/Documents/input_event_inlinehook.ko" at
	.text_addr = 0xffffffffa058a070
(y or n) y
Reading symbols from /home/superman/Documents/input_event_inlinehook.ko...done.
```

之后就可以下断点了：

```
(gdb) b install_hook 

Breakpoint 1 at 0xffffffffa058a140: file /home/superman/Documents/input_event_inlinehook.c, line 104.
```

遇到问题：

调试机现在可下断点，但目标机运行时，在断点处不会断下来。

---

以上方法是断不下来的，现使用下面的方式

```
// 被调试机
[root@localhost Documents]# cat /sys/module/input_event_inlinehook/sections/.text 
0xffffffffa057e000

// 调试机
(gdb) add-symbol-file /home/superman/Documents/input_event_inlinehook.ko 0xffffffffa057e000 -s .bss 0xffffffffa057e5b0 -s .data 0xffffffffa057e2a0
add symbol table from file "/home/superman/Documents/input_event_inlinehook.ko" at
	.text_addr = 0xffffffffa057e000
	.bss_addr = 0xffffffffa057e5b0
	.data_addr = 0xffffffffa057e2a0
(y or n) y
Reading symbols from /home/superman/Documents/input_event_inlinehook.ko...done.
```



**注意**

在gcc和gdb版本不匹配的情况下，可能会出现`Single stepping until exit... which has no line number information`的情况，gcc(4.4)、gdb(7.8)测试可用。