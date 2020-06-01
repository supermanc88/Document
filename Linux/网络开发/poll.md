# poll

```c
       #include <poll.h>

       int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```



```c
struct poolfd{
	int fd;				// file descriptor
	short events;		// requested events
	short revents;		// return events
};
```



1. poll 可以突破1024个文件描述符限制(cat /proc/sys/fs/file-max)
2. 监听、返回集合 分离
3. 搜索范围小

