# task_struct



## 主要成员

- pid

  pid 每个 task_struct 都会有一个不同的 ID，就是这个 PID。

- tid

  tid 线程 ID，用来标识每个线程的。

- tgid

  tgid 线程组领头线程的 PID，事实上就是主线程的 PID。 当创建一个子进程时，它的 tgid 与 pid 相等； 当创建一个线程时，它的 tgid 等于主线程的 pid。

  getpid() 函数事实上返回的是当前进程或线程的 tgid。 

- pgid

  pgid 进程组领头进程的 PID。 

- sid

  sid 会话领头进程的 PID。 

- group_leader

  group_leader 是一个 task_struct 类型的指针，指向的是进程组的组长对应的 task_struct 对象。



sys.c

```c
SYSCALL_DEFINE0(getpid)
{
	return task_tgid_vnr(current);
}

SYSCALL_DEFINE0(getppid)
{
	int pid;

	rcu_read_lock();
	pid = task_tgid_vnr(rcu_dereference(current->real_parent));
	rcu_read_unlock();

	return pid;
}
```

