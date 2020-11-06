# write_begin write_end

路径：mm/filemap.c

generic_perform_write

代码节选：

```c
status = a_ops->write_begin(file, mapping, pos, bytes, flags,
&page, &fsdata);
if (unlikely(status < 0))
break;

if (mapping_writably_mapped(mapping))
flush_dcache_page(page);

copied = iov_iter_copy_from_user_atomic(page, i, offset, bytes);
flush_dcache_page(page);

status = a_ops->write_end(file, mapping, pos, bytes, copied,
page, fsdata);

...
   
balance_dirty_pages_ratelimited(mapping);
```



`write_begin`为写操作准备page页，可以看到`&page`是输入输出类型的。函数由底层的文件系统实现，主要处理需要额外申请的空间，以及从文件中读取不在缓存里的数据，这个函数准备好后，

调用`iov_iter_copy_from_user_atomic`函数将用户层的数据写到`page`中，

之后调用`write_end`更新inode大小，并将`page`标记为`dirty`。

最后调用`balance_dirty_pages_ratelimited`函数平衡内存中的脏页，需要时将脏页刷盘。

