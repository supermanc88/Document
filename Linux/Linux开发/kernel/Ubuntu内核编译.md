# Ubuntu内核编译



## 下载对应的内核源码



先找到对应的内核源码：

> https://wiki.ubuntu.com/Kernel/Dev/KernelGitGuide

|         |                                                              |
| ------- | ------------------------------------------------------------ |
| eoan    | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/eoan |
| disco   | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/disco |
| bionic  | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/bionic |
| xenial  | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/xenial |
| trusty  | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/trusty |
| precise | git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/precise |





根据自己当前的Ubuntu版本下载对应的内核代码。

一般情况下，默认的master分支不是当前的Ubuntu对应的版本代码。

所以需要切换。



查看当前运行的Ubuntu的精确版本号：

> cat /proc/version_signature
>
> Ubuntu 4.15.0-45.48-generic 4.15.18

查找对应的tag

> git tag | grep 4.15.0

切换

> git checkout Ubuntu-4.15.0-45.48

 

## 编译内核



为什么要自己编译呢，主要是因为下载安装Ubuntu的 `'linux-image-'``$(uname ``-``r)``'-dbgsym'`，发现并不能进行调试。

所以还是自己编译的好。还有一个优点就是可以把自己想要调试的驱动直接编译进内核，比调试单独的模块更加简单一些。



编译和安装步骤如下

*a. make menuconfig*

*b. make*

*c. make modules*

*d. make modules_install*

*e. make install*

*f. make headers_install*

注意步**骤d和e**不能颠倒，否则生成的initrd.img不能正常启动，因为它依赖于**步骤c**生成的驱动程序.ko文件。



### 进入编译选项



```sh
make menuconfig
```

正常情况下是不需要改变什么的，需要有

- debug info
- kgdb:use kgdb over the serial console
- kernel debugging



![image-20200901125732043](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901125732043.png)



#### 错误



error1:

```sh
*** Unable to find the ncurses libraries or the
*** required header files.
*** 'make menuconfig' requires the ncurses libraries.
*** Install ncurses (ncurses-devel) and try again.
```

> sudo apt-get install ncurses-dev



error2:

```sh
/bin/sh: 1: flex: not found
scripts/Makefile.lib:194: recipe for target 'scripts/kconfig/lexer.lex.c' failed
make[1]: *** [scripts/kconfig/lexer.lex.c] Error 127
Makefile:552: recipe for target 'menuconfig' failed
make: *** [menuconfig] Error 2
```

> sudo apt-get install flex



error3:

```sh
  LEX     scripts/kconfig/lexer.lex.c
  YACC    scripts/kconfig/parser.tab.h
/bin/sh: 1: bison: not found
scripts/Makefile.lib:208: recipe for target 'scripts/kconfig/parser.tab.h' failed
make[1]: *** [scripts/kconfig/parser.tab.h] Error 127
Makefile:552: recipe for target 'menuconfig' failed
make: *** [menuconfig] Error 2
```

> sudo apt-get install bison



error4:

```sh
scripts/sign-file.c:25:10: fatal error: openssl/opensslv.h: 没有那个文件或目录
 #include <openssl/opensslv.h>
          ^~~~~~~~~~~~~~~~~~~~
compilation terminated.
scripts/Makefile.host:92: recipe for target 'scripts/sign-file' failed
make[1]: *** [scripts/sign-file] Error 1
Makefile:1065: recipe for target 'scripts' failed
make: *** [scripts] Error 2
```

> sudo apt-get install libssl-dev



error5:

```sh
warning: Cannot use CONFIG_STACK_VALIDATION=y, please install libelf-dev, libelf-devel or elfutils-libelf-devel
```

> apt-get install libelf-dev



### 编译



```sh
make
```

**注：虚拟机一般默认是20G大小，这显然是不够的，最好给40G大小**



![image-20200901121401806](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901121401806.png)

![image-20200901121503588](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901121503588.png)

我这是2C2G的虚拟机配置，内核版本5.3.0，编译了将近2小时。



当出现`vmlinux`时，就说明就编译成功了。



### 安装

安装内核模块：

```sh
make modules_install
```

![image-20200901130638749](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901130638749.png)

此命令会将所有的模块安装到内核

驱动驱动会安装到`/lib/modules`

![image-20200901130859919](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901131333515.png)



安装新内核：

```sh
make install
```

![image-20200901131649911](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901130859919.png)

编译的新内核会安装到：`/boot/`

安装之前：

![image-20200901131333515](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901131649911.png)



安装之后：

![image-20200901131720707](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901131830926.png)



并在`/boot/grub/grub.cfg`中更新引导

![image-20200901131830926](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901132042392.png)



默认的引导改为了最新的内核了



如果要修改在开机时进入引导选择，修改`/etc/default/grub`文件：

![image-20200901132042392](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901131720707.png)





## 调试

![image-20200901165952193](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200901165952193.png)



出现这个的原因是：随机化基址

可以在`/boot/grub/grub.cfg`启动参数后加`rodata=off nokaslr`



### gdb调试命令

```sh
set serial baud 115200

target remote /dev/ttyS0
```

