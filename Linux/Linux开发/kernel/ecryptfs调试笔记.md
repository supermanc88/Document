# ecryptfs调试笔记



环境：

linux内核：5.3.13



## ecryptfs_mount



```c
static struct dentry *ecryptfs_mount(struct file_system_type *fs_type, int flags,
			const char *dev_name, void *raw_data)
```



- fs_type: 传进来的是之前定义的新的文件系统类型指针

  ```c
  static struct file_system_type ecryptfs_fs_type = {
  	.owner = THIS_MODULE,
  	.name = "ecryptfs",
  	.mount = ecryptfs_mount,
  	.kill_sb = ecryptfs_kill_block_super,
  	.fs_flags = 0
  };
  ```

- flags：0
- dev_name: `"/home/superman/Desktop/realdir"` 文件系统挂载的路径
- raw_data:`"ecryptfs_sig=cbd6dc63028e5602,ecryptfs_cipher=aes,ecryptfs_key_bytes=16,ecryptfs_unlink_sigs"` 这里是在应用层挂载时，传入的相关参数，如使用的算法，密钥长度等信息





```c
	sbi = kmem_cache_zalloc(ecryptfs_sb_info_cache, GFP_KERNEL);
	if (!sbi) {
		rc = -ENOMEM;
		goto out;
	}
```

分配内存



```c
/* superblock private data. */
struct ecryptfs_sb_info {
	struct super_block *wsi_sb;
	struct ecryptfs_mount_crypt_stat mount_crypt_stat;
};
```



```c
	rc = ecryptfs_parse_options(sbi, raw_data, &check_ruid);
	if (rc) {
		err = "Error parsing options";
		goto out;
	}
```

解析raw_data中传进来的参数:

- 初始化sbi的加密状态
- 解析参数中的信息，填入加密状态结构体中
- 先从一个链表之中，根据当前的加密算法进行查找，如果没有的话，就添加



```c
	mount_crypt_stat = &sbi->mount_crypt_stat;

	s = sget(fs_type, NULL, set_anon_super, flags, NULL);
	if (IS_ERR(s)) {
		rc = PTR_ERR(s);
		goto out;
	}

	rc = super_setup_bdi(s);
	if (rc)
		goto out1;

	ecryptfs_set_superblock_private(s, sbi);
```

`sget`返回一个super_block，当没有存在的时候就创建一个



把super_block_info结构存在sb结构的`s_fs_info`私有区域中



```c
	sbi = NULL;
	s->s_op = &ecryptfs_sops;
	s->s_xattr = ecryptfs_xattr_handlers;
	s->s_d_op = &ecryptfs_dops;
```

填充超级块操作指针等



```c
	err = "Reading sb failed";
	rc = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY, &path);
	if (rc) {
		ecryptfs_printk(KERN_WARNING, "kern_path() failed\n");
		goto out1;
	}
	if (path.dentry->d_sb->s_type == &ecryptfs_fs_type) {
		rc = -EINVAL;
		printk(KERN_ERR "Mount on filesystem of type "
			"eCryptfs explicitly disallowed due to "
			"known incompatibilities\n");
		goto out_free;
	}
```



通过挂载的路径`/home/superman/Desktop/realdir`，使用`kern_path`获得path结构

```sh
(gdb) p *path.dentry
$32 = {d_flags = 2637888, d_seq = {sequence = 2}, d_hash = {next = 0xffff8880b6edb5c8, pprev = 0xffff8880ba784c40}, 
  d_parent = 0xffff8880b33cd980, d_name = {{{hash = 556860248, len = 7}, hash_len = 30621631320}, 
    name = 0xffff8880b0ae09f8 "realdir"}, d_inode = 0xffff8880b0ac5980, d_iname = "realdir", '\000' <repeats 24 times>, d_lockref = {{
      lock_count = 21474836480, {lock = {{rlock = {raw_lock = {{val = {counter = 0}, {locked = 0 '\000', pending = 0 '\000'}, {
                    locked_pending = 0, tail = 0}}}}}}, count = 5}}}, d_op = 0x0 <fixed_percpu_data>, d_sb = 0xffff888034951000, 
  d_time = 0, d_fsdata = 0x0 <fixed_percpu_data>, {d_lru = {next = 0xffff8880b0abcb00, prev = 0xffff8880b0ae1040}, 
    d_wait = 0xffff8880b0abcb00}, d_child = {next = 0xffff8880b0ae1050, prev = 0xffff8880b33cda20}, d_subdirs = {
    next = 0xffff8880b0abd950, prev = 0xffff8880b0abcb10}, d_u = {d_alias = {next = 0x0 <fixed_percpu_data>, 
      pprev = 0xffff8880b0ac5ab8}, d_in_lookup_hash = {next = 0x0 <fixed_percpu_data>, pprev = 0xffff8880b0ac5ab8}, d_rcu = {
      next = 0x0 <fixed_percpu_data>, func = 0xffff8880b0ac5ab8}}}

```



```sh
(gdb) p path.dentry->d_sb->s_type
$33 = (struct file_system_type *) 0xffffffff827224c0 <ext4_fs_type>

```

当前目录项所属的超级块中记录的文件系统类型是ext4类型



```c
	if (check_ruid && !uid_eq(d_inode(path.dentry)->i_uid, current_uid())) {
		rc = -EPERM;
		printk(KERN_ERR "Mount of device (uid: %d) not owned by "
		       "requested user (uid: %d)\n",
			i_uid_read(d_inode(path.dentry)),
			from_kuid(&init_user_ns, current_uid()));
		goto out_free;
	}

	ecryptfs_set_superblock_lower(s, path.dentry->d_sb);


static inline void
ecryptfs_set_superblock_lower(struct super_block *sb,
			      struct super_block *lower_sb)
{
	((struct ecryptfs_sb_info *)sb->s_fs_info)->wsi_sb = lower_sb;
}
```



在当前超级块私有数据中（指向的是sb info），记录下底层的文件系统。（加密文件系统是建立在已有的文件系统之上的）



```c
	/**
	 * Set the POSIX ACL flag based on whether they're enabled in the lower
	 * mount.
	 */
	s->s_flags = flags & ~SB_POSIXACL;
	s->s_flags |= path.dentry->d_sb->s_flags & SB_POSIXACL;

	/**
	 * Force a read-only eCryptfs mount when:
	 *   1) The lower mount is ro
	 *   2) The ecryptfs_encrypted_view mount option is specified
	 */
	if (sb_rdonly(path.dentry->d_sb) || mount_crypt_stat->flags & ECRYPTFS_ENCRYPTED_VIEW_ENABLED)
		s->s_flags |= SB_RDONLY;

	s->s_maxbytes = path.dentry->d_sb->s_maxbytes;
	s->s_blocksize = path.dentry->d_sb->s_blocksize;
	s->s_magic = ECRYPTFS_SUPER_MAGIC;
	s->s_stack_depth = path.dentry->d_sb->s_stack_depth + 1;

	rc = -EINVAL;
	if (s->s_stack_depth > FILESYSTEM_MAX_STACK_DEPTH) {
		pr_err("eCryptfs: maximum fs stacking depth exceeded\n");
		goto out_free;
	}

```

按照ext4超级块中的数据，填充到当前超级块中来

注意`s_stack_depth`这里有`+1`



```c
	inode = ecryptfs_get_inode(d_inode(path.dentry), s);
	rc = PTR_ERR(inode);
	if (IS_ERR(inode))
		goto out_free;

	s->s_root = d_make_root(inode);
	if (!s->s_root) {
		rc = -ENOMEM;
		goto out_free;
	}


static struct inode *__ecryptfs_get_inode(struct inode *lower_inode,
					  struct super_block *sb)
{
	struct inode *inode;

	if (lower_inode->i_sb != ecryptfs_superblock_to_lower(sb))
		return ERR_PTR(-EXDEV);
	if (!igrab(lower_inode))
		return ERR_PTR(-ESTALE);
	inode = iget5_locked(sb, (unsigned long)lower_inode,
			     ecryptfs_inode_test, ecryptfs_inode_set,
			     lower_inode);
	if (!inode) {
		iput(lower_inode);
		return ERR_PTR(-EACCES);
	}
	if (!(inode->i_state & I_NEW))
		iput(lower_inode);

	return inode;
}


static int ecryptfs_inode_set(struct inode *inode, void *opaque)
{
	struct inode *lower_inode = opaque;

	ecryptfs_set_inode_lower(inode, lower_inode);
	fsstack_copy_attr_all(inode, lower_inode);
	/* i_size will be overwritten for encrypted regular files */
	fsstack_copy_inode_size(inode, lower_inode);
	inode->i_ino = lower_inode->i_ino;
	inode->i_mapping->a_ops = &ecryptfs_aops;

	if (S_ISLNK(inode->i_mode))
		inode->i_op = &ecryptfs_symlink_iops;
	else if (S_ISDIR(inode->i_mode))
		inode->i_op = &ecryptfs_dir_iops;
	else
		inode->i_op = &ecryptfs_main_iops;

	if (S_ISDIR(inode->i_mode))
		inode->i_fop = &ecryptfs_dir_fops;
	else if (special_file(inode->i_mode))
		init_special_inode(inode, inode->i_mode, inode->i_rdev);
	else
		inode->i_fop = &ecryptfs_main_fops;

	return 0;
}
```

`ecryptfs_get_inode`函数传入的参数是当前目录项在ext4上的inode结构，和当前加密文件系统的超级块，是为了在当前加密文件系统超级块上伪造一个和ext4上一样的inode



内部调用`__ecryptfs_get_inode`函数创建了一个新的节点，并通过`ecryptfs_inode_set`给新inode节点设置了相关的操作函数及文件操作相关函数。



```c
	rc = -ENOMEM;
	root_info = kmem_cache_zalloc(ecryptfs_dentry_info_cache, GFP_KERNEL);
	if (!root_info)
		goto out_free;

	/* ->kill_sb() will take care of root_info */
	ecryptfs_set_dentry_private(s->s_root, root_info);
	root_info->lower_path = path;

	s->s_flags |= SB_ACTIVE;
	return dget(s->s_root);


static inline void
ecryptfs_set_dentry_private(struct dentry *dentry,
			    struct ecryptfs_dentry_info *dentry_info)
{
	dentry->d_fsdata = dentry_info;
}
```



把自建dentry info结构体记录到dentry私有结构上。

自建目录项信息记录底层path结构



```c
out_free:
	path_put(&path);
out1:
	deactivate_locked_super(s);
out:
	if (sbi) {
		ecryptfs_destroy_mount_crypt_stat(&sbi->mount_crypt_stat);
		kmem_cache_free(ecryptfs_sb_info_cache, sbi);
	}
	printk(KERN_ERR "%s; rc = [%d]\n", err, rc);
	return ERR_PTR(rc);
```

失败清理工作



以上mount函数分析完毕



## 文件操作

```c
 const struct file_operations ecryptfs_dir_fops = {
 	.iterate_shared = ecryptfs_readdir,
 	.read = generic_read_dir,
 	.unlocked_ioctl = ecryptfs_unlocked_ioctl,
 #ifdef CONFIG_COMPAT
 	.compat_ioctl = ecryptfs_compat_ioctl,
 #endif
 	.open = ecryptfs_dir_open,
 	.release = ecryptfs_dir_release,
 	.fsync = ecryptfs_fsync,
 	.llseek = ecryptfs_dir_llseek,
 };
 
 const struct file_operations ecryptfs_main_fops = {
 	.llseek = generic_file_llseek,
 	.read_iter = ecryptfs_read_update_atime,
 	.write_iter = generic_file_write_iter,
 	.unlocked_ioctl = ecryptfs_unlocked_ioctl,
 #ifdef CONFIG_COMPAT
 	.compat_ioctl = ecryptfs_compat_ioctl,
 #endif
 	.mmap = ecryptfs_mmap,
 	.open = ecryptfs_open,
 	.flush = ecryptfs_flush,
 	.release = ecryptfs_release,
 	.fsync = ecryptfs_fsync,
 	.fasync = ecryptfs_fasync,
 	.splice_read = generic_file_splice_read,
 };
```

在linux内核中，文件的操作由一个`file_operations`结构体表示，其中成员是各种文件操作函数指针。



### ecryptfs_open



```c
static int ecryptfs_open(struct inode *inode, struct file *file)
```



- inode: 对应文件的inode节点
- file: 对应的file结构

```sh
Thread 665 hit Breakpoint 1, ecryptfs_open (inode=0xffff8880a16e9a40, file=0xffff88809669a500) at fs/ecryptfs/file.c:192
192	{
(gdb) p *file
$1 = {f_u = {fu_llist = {next = 0x0 <fixed_percpu_data>}, fu_rcuhead = {next = 0x0 <fixed_percpu_data>, 
      func = 0x0 <fixed_percpu_data>}}, f_path = {mnt = 0xffff8880b0307720, dentry = 0xffff8880b63d5140}, 
  f_inode = 0xffff8880a16e9a40, f_op = 0xffffffff82063760 <ecryptfs_main_fops>, f_lock = {{rlock = {raw_lock = {{val = {counter = 0}, {
              locked = 0 '\000', pending = 0 '\000'}, {locked_pending = 0, tail = 0}}}}}}, f_write_hint = WRITE_LIFE_NOT_SET, 
  f_count = {counter = 1}, f_flags = 32768, f_mode = 32797, f_pos_lock = {owner = {counter = 0}, wait_lock = {{rlock = {raw_lock = {{
              val = {counter = 0}, {locked = 0 '\000', pending = 0 '\000'}, {locked_pending = 0, tail = 0}}}}}}, osq = {tail = {
        counter = 0}}, wait_list = {next = 0xffff88809669a558, prev = 0xffff88809669a558}}, f_pos = 0, f_owner = {lock = {raw_lock = {{
          cnts = {counter = 0}, {wlocked = 0 '\000', __lstate = "\000\000"}}, wait_lock = {{val = {counter = 0}, {locked = 0 '\000', 
              pending = 0 '\000'}, {locked_pending = 0, tail = 0}}}}}, pid = 0x0 <fixed_percpu_data>, pid_type = PIDTYPE_PID, uid = {
      val = 0}, euid = {val = 0}, signum = 0}, f_cred = 0xffff8880a9888600, f_ra = {start = 0, size = 0, async_size = 0, ra_pages = 0, 
    mmap_miss = 0, prev_pos = 0}, f_version = 0, f_security = 0xffff8880966f43f0, private_data = 0x0 <fixed_percpu_data>, 
  f_ep_links = {next = 0xffff88809669a5d0, prev = 0xffff88809669a5d0}, f_tfile_llink = {next = 0xffff88809669a5e0, 
    prev = 0xffff88809669a5e0}, f_mapping = 0xffff8880a16e9bb0, f_wb_err = 0}

```





```c
	int rc = 0;
	struct ecryptfs_crypt_stat *crypt_stat = NULL;
	struct dentry *ecryptfs_dentry = file->f_path.dentry;
	/* Private value of ecryptfs_dentry allocated in
	 * ecryptfs_lookup() */
	struct ecryptfs_file_info *file_info;

	/* Released in ecryptfs_release or end of function if failure */
	file_info = kmem_cache_zalloc(ecryptfs_file_info_cache, GFP_KERNEL);
	ecryptfs_set_file_private(file, file_info);
	if (!file_info) {
		ecryptfs_printk(KERN_ERR,
				"Error attempting to allocate memory\n");
		rc = -ENOMEM;
		goto out;
	}
```

每打开一个文件便创建一个自建文件信息结构体，将该结构体和file结构的私有数据关联上。



```c
	crypt_stat = &ecryptfs_inode_to_private(inode)->crypt_stat;
	mutex_lock(&crypt_stat->cs_mutex);
// 新建的节点加密状态是空的，第一次打开时，会给设置上加密标志
	if (!(crypt_stat->flags & ECRYPTFS_POLICY_APPLIED)) {
		ecryptfs_printk(KERN_DEBUG, "Setting flags for stat...\n");
		/* Policy code enabled in future release */
		crypt_stat->flags |= (ECRYPTFS_POLICY_APPLIED
				      | ECRYPTFS_ENCRYPTED);
	}
	mutex_unlock(&crypt_stat->cs_mutex);

static inline struct ecryptfs_inode_info *
ecryptfs_inode_to_private(struct inode *inode)
{
	return container_of(inode, struct ecryptfs_inode_info, vfs_inode);
}
```



#### inode和ecryptfs_inode_info关系解释

需要看下`ecryptfs_inode_to_private`为什么`inode`地址会和`ecryptfs_inode_info`地址相关？

在`ecryptfs_mount`函数中有这么一行代码：

```c
s->s_op = &ecryptfs_sops;

// ecryptfs_sops具体内容：
 const struct super_operations ecryptfs_sops = {
 	.alloc_inode = ecryptfs_alloc_inode,
 	.destroy_inode = ecryptfs_destroy_inode,
 	.free_inode = ecryptfs_free_inode,
 	.statfs = ecryptfs_statfs,
 	.remount_fs = NULL,
 	.evict_inode = ecryptfs_evict_inode,
 	.show_options = ecryptfs_show_options
 };

```



在`ecryptfs_alloc_inode`函数中有相关代码：

```c
 static struct inode *ecryptfs_alloc_inode(struct super_block *sb)
 {
 	struct ecryptfs_inode_info *inode_info;
 	struct inode *inode = NULL;
 
 	inode_info = kmem_cache_alloc(ecryptfs_inode_info_cache, GFP_KERNEL);
 	if (unlikely(!inode_info))
 		goto out;
 	if (ecryptfs_init_crypt_stat(&inode_info->crypt_stat)) {
 		kmem_cache_free(ecryptfs_inode_info_cache, inode_info);
 		goto out;
 	}
 	mutex_init(&inode_info->lower_file_mutex);
 	atomic_set(&inode_info->lower_file_count, 0);
 	inode_info->lower_file = NULL;
 	inode = &inode_info->vfs_inode;
 out:
 	return inode;
 }
```

在这个函数中，会直接创建一个`inode_info`，其中的`vfs_inode`就是文件操作中所使用的`inode`节点。



```c
 	rc = ecryptfs_get_lower_file(ecryptfs_dentry, inode);
 	if (rc) {
 		printk(KERN_ERR "%s: Error attempting to initialize "
 			"the lower file for the dentry with name "
 			"[%pd]; rc = [%d]\n", __func__,
 			ecryptfs_dentry, rc);
 		goto out_free;
 	}
// lower_file是file*类型，f_flags是文件打开时的标志，这是比较底层文件打开是只读，当前打开标志不是只读，则退出
 	if ((ecryptfs_inode_to_private(inode)->lower_file->f_flags & O_ACCMODE)
 	    == O_RDONLY && (file->f_flags & O_ACCMODE) != O_RDONLY) {
 		rc = -EPERM;
 		printk(KERN_WARNING "%s: Lower file is RO; eCryptfs "
 		       "file must hence be opened RO\n", __func__);
 		goto out_put;
 	}
 	ecryptfs_set_file_lower(
 		file, ecryptfs_inode_to_private(inode)->lower_file);
 	rc = read_or_initialize_metadata(ecryptfs_dentry);
 	if (rc)
 		goto out_put;
 	ecryptfs_printk(KERN_DEBUG, "inode w/ addr = [0x%p], i_ino = "
 			"[0x%.16lx] size: [0x%.16llx]\n", inode, inode->i_ino,
 			(unsigned long long)i_size_read(inode));
 	goto out;
 out_put:
 	ecryptfs_put_lower_file(inode);
 out_free:
 	kmem_cache_free(ecryptfs_file_info_cache,
 			ecryptfs_file_to_private(file));
 out:
 	return rc;
```



`ecryptfs_get_lower_file`通过`ecryptfs_dentry`来填充`inode`中的`lower_file`结构

并通过`ecryptfs_set_file_lower`将`file`的私有数据指针指向下层的file结构

`read_or_initialize_metadata` 函数读取文件的元数据，如文件标题信息、属性等



#### read_or_initialize_metadata

```c
 static int read_or_initialize_metadata(struct dentry *dentry)
 {
 	struct inode *inode = d_inode(dentry);
 	struct ecryptfs_mount_crypt_stat *mount_crypt_stat;
 	struct ecryptfs_crypt_stat *crypt_stat;
 	int rc;
 
 	crypt_stat = &ecryptfs_inode_to_private(inode)->crypt_stat;
     // 这是获取了加密文件系统的超级块加密状态
 	mount_crypt_stat = &ecryptfs_superblock_to_private(
 						inode->i_sb)->mount_crypt_stat;
 	mutex_lock(&crypt_stat->cs_mutex);
 
 	if (crypt_stat->flags & ECRYPTFS_POLICY_APPLIED &&
 	    crypt_stat->flags & ECRYPTFS_KEY_VALID) {
 		rc = 0;
 		goto out;
 	}
 
 	rc = ecryptfs_read_metadata(dentry);
 	if (!rc)
 		goto out;
 
 	if (mount_crypt_stat->flags & ECRYPTFS_PLAINTEXT_PASSTHROUGH_ENABLED) {
 		crypt_stat->flags &= ~(ECRYPTFS_I_SIZE_INITIALIZED
 				       | ECRYPTFS_ENCRYPTED);
 		rc = 0;
 		goto out;
 	}
 
 	if (!(mount_crypt_stat->flags & ECRYPTFS_XATTR_METADATA_ENABLED) &&
 	    !i_size_read(ecryptfs_inode_to_lower(inode))) {
 		rc = ecryptfs_initialize_file(dentry, inode);
 		if (!rc)
 			goto out;
 	}
 
 	rc = -EIO;
 out:
 	mutex_unlock(&crypt_stat->cs_mutex);
 	return rc;
 }
```



#### ecryptfs_read_metadata分析



### ecryptfs_release

```c
 static int ecryptfs_release(struct inode *inode, struct file *file)
 {
 	ecryptfs_put_lower_file(inode);
 	kmem_cache_free(ecryptfs_file_info_cache,
 			ecryptfs_file_to_private(file));
 	return 0;
 }
```





### ecryptfs_dir_open

```c
 static int ecryptfs_dir_open(struct inode *inode, struct file *file)
 {
 	struct dentry *ecryptfs_dentry = file->f_path.dentry;
 	/* Private value of ecryptfs_dentry allocated in
 	 * ecryptfs_lookup() */
 	struct ecryptfs_file_info *file_info;
 	struct file *lower_file;
 
 	/* Released in ecryptfs_release or end of function if failure */
 	file_info = kmem_cache_zalloc(ecryptfs_file_info_cache, GFP_KERNEL);
 	ecryptfs_set_file_private(file, file_info);
 	if (unlikely(!file_info)) {
 		ecryptfs_printk(KERN_ERR,
 				"Error attempting to allocate memory\n");
 		return -ENOMEM;
 	}
 	lower_file = dentry_open(ecryptfs_dentry_to_lower_path(ecryptfs_dentry),
 				 file->f_flags, current_cred());
 	if (IS_ERR(lower_file)) {
 		printk(KERN_ERR "%s: Error attempting to initialize "
 			"the lower file for the dentry with name "
 			"[%pd]; rc = [%ld]\n", __func__,
 			ecryptfs_dentry, PTR_ERR(lower_file));
 		kmem_cache_free(ecryptfs_file_info_cache, file_info);
 		return PTR_ERR(lower_file);
 	}
 	ecryptfs_set_file_lower(file, lower_file);
 	return 0;
 }
```



核心就是`dentry_open`函数打开底层的目录项



### ecryptfs_fasync

```c
 static int ecryptfs_fasync(int fd, struct file *file, int flag)
 {
 	int rc = 0;
 	struct file *lower_file = NULL;
 
 	lower_file = ecryptfs_file_to_lower(file);
 	if (lower_file->f_op->fasync)
 		rc = lower_file->f_op->fasync(fd, lower_file, flag);
 	return rc;
 }
```

该函数用于打或关闭异步I/O的通告信号

### ecryptfs_fsync

```c
 static int
 ecryptfs_fsync(struct file *file, loff_t start, loff_t end, int datasync)
 {
 	int rc;
 
 	rc = file_write_and_wait(file);
 	if (rc)
 		return rc;
 
 	return vfs_fsync(ecryptfs_file_to_lower(file), datasync);
 }
```

将给定文件的所有被缓存的数据写回磁盘。由系统调用。



## 超级块操作



```c
 const struct super_operations ecryptfs_sops = {
 	.alloc_inode = ecryptfs_alloc_inode,
 	.destroy_inode = ecryptfs_destroy_inode,
 	.free_inode = ecryptfs_free_inode,
 	.statfs = ecryptfs_statfs,
 	.remount_fs = NULL,
 	.evict_inode = ecryptfs_evict_inode,
 	.show_options = ecryptfs_show_options
 };
```





### ecryptfs_alloc_inode

```c
 static struct inode *ecryptfs_alloc_inode(struct super_block *sb)
 {
 	struct ecryptfs_inode_info *inode_info;
 	struct inode *inode = NULL;
 
 	inode_info = kmem_cache_alloc(ecryptfs_inode_info_cache, GFP_KERNEL);
 	if (unlikely(!inode_info))
 		goto out;
 	if (ecryptfs_init_crypt_stat(&inode_info->crypt_stat)) {
 		kmem_cache_free(ecryptfs_inode_info_cache, inode_info);
 		goto out;
 	}
 	mutex_init(&inode_info->lower_file_mutex);
 	atomic_set(&inode_info->lower_file_count, 0);
 	inode_info->lower_file = NULL;
 	inode = &inode_info->vfs_inode;
 out:
 	return inode;
 }
```



```c
 /* inode private data. */
 struct ecryptfs_inode_info {
 	struct inode vfs_inode;
 	struct inode *wii_inode;
 	struct mutex lower_file_mutex;
 	atomic_t lower_file_count;
 	struct file *lower_file;
 	struct ecryptfs_crypt_stat crypt_stat;
 };
```



每创建一个inode节点，便申请分配一个`ecryptfs_inode_info`结构体



注意`inode_info->lower_file = NULL;`这里，`lower_file`并未设置，在`ecryptfs_open`中会被设置。



## 节点操作



```c
 const struct inode_operations ecryptfs_symlink_iops = {
 	.get_link = ecryptfs_get_link,
 	.permission = ecryptfs_permission,
 	.setattr = ecryptfs_setattr,
 	.getattr = ecryptfs_getattr_link,
 	.listxattr = ecryptfs_listxattr,
 };
 
 const struct inode_operations ecryptfs_dir_iops = {
 	.create = ecryptfs_create,
 	.lookup = ecryptfs_lookup,
 	.link = ecryptfs_link,
 	.unlink = ecryptfs_unlink,
 	.symlink = ecryptfs_symlink,
 	.mkdir = ecryptfs_mkdir,
 	.rmdir = ecryptfs_rmdir,
 	.mknod = ecryptfs_mknod,
 	.rename = ecryptfs_rename,
 	.permission = ecryptfs_permission,
 	.setattr = ecryptfs_setattr,
 	.listxattr = ecryptfs_listxattr,
 };
 
 const struct inode_operations ecryptfs_main_iops = {
 	.permission = ecryptfs_permission,
 	.setattr = ecryptfs_setattr,
 	.getattr = ecryptfs_getattr,
 	.listxattr = ecryptfs_listxattr,
 };
```



### ecryptfs_create

```c
 static int
 ecryptfs_create(struct inode *directory_inode, struct dentry *ecryptfs_dentry,
 		umode_t mode, bool excl)
 {
 	struct inode *ecryptfs_inode;
 	int rc;
 
 	ecryptfs_inode = ecryptfs_do_create(directory_inode, ecryptfs_dentry,
 					    mode);
 	if (IS_ERR(ecryptfs_inode)) {
 		ecryptfs_printk(KERN_WARNING, "Failed to create file in"
 				"lower filesystem\n");
 		rc = PTR_ERR(ecryptfs_inode);
 		goto out;
 	}
 	/* At this point, a file exists on "disk"; we need to make sure
 	 * that this on disk file is prepared to be an ecryptfs file */
 	rc = ecryptfs_initialize_file(ecryptfs_dentry, ecryptfs_inode);
 	if (rc) {
 		ecryptfs_do_unlink(directory_inode, ecryptfs_dentry,
 				   ecryptfs_inode);
 		iget_failed(ecryptfs_inode);
 		goto out;
 	}
 	d_instantiate_new(ecryptfs_dentry, ecryptfs_inode);
 out:
 	return rc;
 }
```

VFS通过系统调用`create`和`open`来调用该函数，从而为dentry对象创建一个新的索引节点。在创建时使用mode指定的初始模式。



为什么超级块上已经有alloc_inode的功能了，inode操作函数还要有创建inode的函数：

其实在内部调用了：

```c
	inode = iget5_locked(sb, (unsigned long)lower_inode,
			     ecryptfs_inode_test, ecryptfs_inode_set,
```





此函数在内部调用了`vfs_create`调用底层文件系统进行创建inode。之后进行初始化。`ecryptfs_initialize_file`函数内部对inode进行加密状态的初始化，密钥，ctx等初始化操作后，用` ecryptfs_write_metadata`函数对节点或目录项写入元数据。



最后创建一个新的缓存。







