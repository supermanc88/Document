# Linux 内核编译全记录

环境：Ubuntu 16.04
内核版本： 4.15.0-88-generic


## 下载源码

由于是第一次编译内核，就找一个和目前版本相近的版本代码来编译

在[kernel](www.kernel.org)下载源码，我这里找的是4.14.172版本

将源码放置到`/usr/src`目录下，并解压

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583388103(1).jpg)

创建一个软链接

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583388683(1).jpg)


## 配置

Linux内核有多种方式去编译：

- make config (基于文本的最为传统的配置界面，不推荐使用)

- make menuconfig (基于文本菜单的配置界面)

- make xconfig (要求了QT开发环境)

- make kconfig (KDE桌面环境，并且安装了QT开发环境)

- make gconfig (gnome桌面环境，并且安装了QT开发环境)



一般都是用`make menuconfig`的方式去编译

如下所示，选择配置：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583389138(1).jpg)

目前来看，我什么也不知道，就先按照默认的来编译吧，选择save并退出

### 内核调试选项

选择 `Kernel hacking`中的内容：

```
Kernel hacking--->  
       -*- Magic SysRq key  
       [*] Kernel debugging  
       [*] Compile the kernel with debug info  
       [*] KGDB: kernel debugging with remote gdb --->  
              <*> KGDB: use kgdb over the serial console  
```


## 编译

在配置完成后，就可以编译了，我使用的是虚拟机，给了两个核心：

```
make -j2 # 其中的2是编译时所使用的线程数
```

编译过程中出现一个错误

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583396226(1).jpg)

原因是虚拟机20G的大小全给用完了，可能是我别的软件装大多的原因，虚拟机关机扩容


扩容完毕之后，重新make编译

50分钟过去，编成功了：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583392301(1).jpg)


## 安装

### 安装模块：

```
make modules_install
```

安装完成后，会在`/lib/modules`目录下生成对应版本的目录

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584632963(1).jpg)


### 内核安装

这是在内核install之前的情况：

建议备份initrd和vmlinuz，虽然他们会自己备份。

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584633211(1).png)

再看看源码目录,多出了个vmlinux文件：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583396589(1).jpg)


```
make install
```

执行make install，将会拷贝源码目录下的vmlinux到/boot/目录并压缩为vmlinuz，并创建initrd

安装完成后会在`/boot/`目录下生成对应版本的目录

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583396860(1).jpg)

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584633351(1).png)

一切没问题后，查看一下`/boot/grub/grub.conf`

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584634467(1).jpg)


修改`/etc/default/grub`

注释掉 `GRUB_HIDDEN_TIMEOUT`

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583398656(1).jpg)


修改之后，必须运行：

```
sudo update-grub
```

#### 为KGDB内核创建新的启动项

新增的KGDB启动项，与原始启动项相比只增加了一个参数：`kgdboc=ttyS0,11520`

如果需要目标机一启动就断住（比如要调试启动阶段的代码），则再增加一个参数`kgdbwait`

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1583397078(1).jpg)

修改之后，必须运行：

```
sudo update-grub
```

修改已更新：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-8b728aaa77aefedc1a51797525486c17_720w.jpg)

重启系统时就可以选择内核了


## 内核调试虚拟机配置

### 调试环境一

主机：manjaro 

被调试机：虚拟机ubuntu

虚拟机配置串口：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584634592(1).png)

自定义名为： `/tmp/isocket`

其中/tmp/isocket是一个文件，主机和虚拟机利用这个文件进行串口通信。

主机需要两个程序socat和telnet

在主机terminal里：

```shell
socat /tmp/isocket tcp-listen:9001 &
telnet 127.0.0.1 9001
```

在虚拟机上：

```shell
cat /dev/ttyS0
```

此时主机terminal里输入信息虚拟机可以通过ttyS0接收到。


#echo ‘hello’ > /dev/ttyS0

主机的terminal可以收到

telnet输入Ctrl+]结束

如果串口收发没有问题，可以继续。

然后在主机terminal中

```shell
socat /tmp/isocket tcp-listen:9001 &

cd <vmlinux所在目录 >

gdb vmlinux

(gdb) target remote:9001
```

正常情况下就可以调试了

在这里输入c就会把控制权交还给虚拟机，当我们想把控制权交还给主机的gdb是，在虚拟机的terminal中输入

```shell
echo g>/proc/sysrq-trigger
```


### 调试环境二

主机：虚拟机ubuntu 最好使用14.0.4 高版本的编译的时候没有frame pointer选项，调试一直有问题，断点打不上

被调试机：虚拟机buuntu

两个虚拟机为编译出内核后，克隆而来


主机配置：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584779659.png)

该端是客户端

被调试机配置：

![image](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200812115534546.png)

该端是服务器

开机之后，被调试机和之前一样进入等待调试阶段，



```sh
echo g>/proc/sysrq-trigger
```



主机使用如下命令：

```
gdb --baud 11520 vmlinux
target remote /dev/ttyS0
```

开始调试。





### CentOS安装的debuginfo



路径如下：



![image-20200812115534546](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1584779604(1).png)



centos系统中debuginfo查找地址：

http://debuginfo.centos.org/