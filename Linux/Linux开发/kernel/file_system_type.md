# file_system_type

此结构体用于文件系统的注册



这里的文件系统是指可能会被挂载到目录树中的各个实际文件系统，所谓实际文件系统，即是指VFS 中的实际操作最终要通过它们来完成而已，并不意味着它们一定要存在于某种特定的存储设备上。比如在笔者的 Linux 机器下就注册有 "rootfs"、"proc"、"ext2"、"sockfs" 等十几种文件系统。



```c
struct file_system_type {
	const char *name;
	int fs_flags;
	int (*get_sb) (struct file_system_type *, int,
		       const char *, void *, struct vfsmount *);
	void (*kill_sb) (struct super_block *);
	struct module *owner;
	struct file_system_type * next;
	struct list_head fs_supers;

	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;

	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key i_mutex_dir_key;
	struct lock_class_key i_alloc_sem_key;
};
```



注册过程实际上将表示各实际文件系统的 `struct file_system_type `数据结构的实例化，然后形成一个链表，内核中用一个名为 `file_systems` 的全局变量来指向该链表的表头。



- name：文件系统的名字，这个名字唯一的标识一种文件系统；
- next：为文件系统的链表指针；
- fs_supers：对于每一个mount的文件系统，系统都会为它创建一个super_block数据结构，该结构保存文件系统本身以及挂载点相关的信息。由于可以同时挂载多个同一文件系统类型的文件系统（比如/ 和/home都挂载了ext3文件系统），因此同一个文件系统类型会对应多个super block，@fs_supers就把这个文件系统类型对应的super block链接起来。
- owner是指向module的指针，仅当文件系统类型是以模块方式注册时，owner才有效。



