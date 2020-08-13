# Linux文件系统



## 目录

- /bin

  基本命令

- sbin

  系统命令

- /dev

  设备文件存储目录，应用程序通过对这些文件的读写和控制以访问实际的设备

- /etc

  系统配置文件的所在地，一些服务器的配置文件也在这里

- /lib

  系统库文件存放目录

- /mnt

  一般是用于存放挂载储存设备的挂载目录

- /opt

  有此软件包会被安装在这里

- /proc

  操作系统运行时，进程及内核信息（比如CPU、硬盘分区、内存信息等）存放在这里。proc目录为伪文件系统proc的挂载目录，proc并不是真正的文件系统，它存在于内核之中。

- /tmp

  用户运行程序的时候，有时会产生临时文件，/tmp用来存放临时文件

- /usr

  系统存放程序的目录

- /var

  /var/log目录经常存放系统目录

- /sys

  2.6内核之后的内核所支持的sysfs文件系统被映射在此目录上。Linux设备驱动模型中的总线、驱动和设备都可以在sysfs文件系统中找到对应的节点。当内核检测到在系统中出现了新设备后，内核会在sysfs文件系统中为该新设备生成一项新的记录。



## Linux文件系统与设备驱动



应用程序和VFS之间的接口是系统调用。



VFS与文件系统以及设备文件之间的接口是`file_operations`结构体成员函数。这个结构体包含对文件进行打开、关闭、读写、控制的一系统函数。



通过文件系统来访问块设备，`file_operations`的实现则位于文件系统内，文件系统会把针对文件的读写转换为针对块设备原始扇区的读写。





## VFS对象及其数据结构

VFS中有四个主要的对象类型，它们分别是：

- 超级块对象：它代表一个具体的已安装文件系统
- 索引节点对象：它代表一个具体文件
- 目录项对象：它代表一个目录项，是路径组成的一部分
- 文件对象：它代表由进程打开的文件



## get_sb分析



>The get_sb() method has the following arguments:
>  struct file_system_type *fs_type: describes the filesystem, partly initialized by the specific filesystem code
>  int flags: mount flags
>  const char *dev_name: the device name we are mounting.
>  void *data: arbitrary mount options, usually comes as an ASCII string (see "Mount Options" section)
>  struct vfsmount *mnt: a vfs-internal representation of a mount point



共有5个参数：

- fs_type 
- flags 挂载标志
- dev_name 将要挂载的设备名
- data 挂载选项
- mnt 一个挂载点的vfs内部表示，一个系统实例



get_sb（）方法必须确定dev_name和fs_type中指定的块设备是否包含该方法支持的文件系统。如果成功打开命名的块设备，它将为该块设备所包含的文件系统初始化一个struct super_block描述符。失败时返回错误。

get_sb（）方法填充的超级块结构中最有趣的成员是“ s_op”字段。这是指向“ struct super_operations”的指针，该指针描述了文件系统实现的下一级。



## get_sb_nodev 函数分析



通常，文件系统使用一个通用的get_sb()实现，并提供一个fill_super()方法。



- get_sb_bdev：安装在块设备上的文件系统

- get_sb_nodev：安装不受设备支持的文件系统

- get_sb_single：安装在以下设备之间共享实例的文件系统



```c
int get_sb_nodev(struct file_system_type *fs_type,
	int flags, void *data,
	int (*fill_super)(struct super_block *, void *, int),
	struct vfsmount *mnt)
{
	int error;
	struct super_block *s = sget(fs_type, NULL, set_anon_super, NULL);

	if (IS_ERR(s))
		return PTR_ERR(s);

	s->s_flags = flags;

	error = fill_super(s, data, flags & MS_SILENT ? 1 : 0);
	if (error) {
		deactivate_locked_super(s);
		return error;
	}
	s->s_flags |= MS_ACTIVE;
	simple_set_mnt(mnt, s);
	return 0;
}
```



fill_super 是由开发者提供的sb填充的函数指针。



fill_super函数主要填充的sb结构体中的字段有：

- s_fs_info 文件系统特殊信息，可以存放一些自己构造的数据
- s_op 超级块操作方法
- s_root 目录挂载点，`struct dentry *`类型，使用`d_alloc`函数分配
- s_root->d_op 目录项操作指针
- s_root->d_sb 文件所属超级块，就是自己
- s_root->d_parent 父目录的目录项对象
- s_root->d_fsdata 分配内存，存放文件系统的特有数据







