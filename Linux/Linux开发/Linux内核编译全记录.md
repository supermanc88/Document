# Linux 内核编译全记录

环境：Ubuntu 16.04
内核版本： 4.15.0-88-generic


## 下载源码

由于是第一次编译内核，就找一个和目前版本相近的版本代码来编译

在[kernel](www.kernel.org)下载源码，我这里找的是4.14.172版本

将源码放置到`/usr/src`目录下，并解压

![image](./images/1583388103(1).jpg)

创建一个软链接

![image](./images/1583388683(1).jpg)


## 配置

Linux内核有多种方式去编译：

- make config (基于文本的最为传统的配置界面，不推荐使用)

- make menuconfig (基于文本菜单的配置界面)

- make xconfig (要求了QT开发环境)

- make kconfig (KDE桌面环境，并且安装了QT开发环境)

- make gconfig (gnome桌面环境，并且安装了QT开发环境)



一般都是用`make menuconfig`的方式去编译

如下所示，选择配置：

![image](./images/1583389138(1).jpg)

目前来看，我什么也不知道，就先按照默认的来编译吧，选择save并退出


## 编译

在配置完成后，就可以编译了，我使用的是虚拟机，给了两个核心：

```
make -j2 # 其中的2是编译时所使用的线程数
```

编译过程中出现一个错误

![image](./images/1583392301(1).jpg)

原因是虚拟机20G的大小全给用完了，可能是我别的软件装大多的原因，虚拟机关机扩容


扩容完毕之后，重新make编译

50分钟过去，编成功了：

![image](./images/1583396226(1).jpg)


## 安装

安装模块：

```
make modules_install
```

安装完成后，会在`/lib/modules`目录下生成对应版本的目录

![image](./images/1583396589(1).jpg)


内核安装

```
make install
```

安装完成后会在`/boot/`目录下生成对应版本的目录

![image](./images/1583396860(1).jpg)

一切没问题后，查看一下`/boot/grub/grub.conf`

![image](./images/1583397078(1).jpg)


修改`/etc/default/grub`

注释掉 `GRUB_HIDDEN_TIMEOUT`

![image](./images/1583398656(1).jpg)


修改之后，必须运行：

```
sudo update-grub
```

重启系统时就可以选择内核了





