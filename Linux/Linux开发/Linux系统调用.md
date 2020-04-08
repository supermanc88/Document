# Linux系统调用分析


以 linux-2.6.32.69 源码分析


```
#define __SYSCALL(nr, sym) extern asmlinkage void sym(void) ;


typedef void (*sys_call_ptr_t)(void);

const sys_call_ptr_t sys_call_table[__NR_syscall_max+1] = {
	/*
	*Smells like a like a compiler bug -- it doesn't work
	*when the & below is removed.
	*/
	[0 ... __NR_syscall_max] = &sys_ni_syscall,
#include <asm/unistd_64.h>
};
```

```
asmlinkage long sys_ni_syscall(void)
{
	return -ENOSYS;
}
```

上面的代码就是给sys_call_table设置一个初始化的值，这个函数指针指向一个返回没有系统调用的函数。


可以看到`sys_call_table`中存放的是函数指针。

`unistd_64.h`文件中的代码如下：

```
#define __NR_read				0
__SYSCALL(__NR_read, sys_read)
#define __NR_write				1
__SYSCALL(__NR_write, sys_write)
#define __NR_open				2
__SYSCALL(__NR_open, sys_open)
#define __NR_close				3
__SYSCALL(__NR_close, sys_close)
#define __NR_stat				4
__SYSCALL(__NR_stat, sys_newstat)
#define __NR_fstat				5
__SYSCALL(__NR_fstat, sys_newfstat)
#define __NR_lstat				6
__SYSCALL(__NR_lstat, sys_newlstat)
#define __NR_poll				7
__SYSCALL(__NR_poll, sys_poll)

#define __NR_lseek				8
__SYSCALL(__NR_lseek, sys_lseek)
#define __NR_mmap				9
__SYSCALL(__NR_mmap, sys_mmap)
...
```