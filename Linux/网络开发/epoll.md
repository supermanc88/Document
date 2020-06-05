# epoll



## epoll

是Linux下多路复用IO接口select/poll的增强版本。



epoll 可以找到监控的哪个文件描述符被激活了，不用像select和poll那样去遍历



当连接的描述符比较多，但监听的比较少，推荐使用epoll



- 向Linux内核建议要创建多大的红黑树，实际使用时可以超过这个值

```c
       #include <sys/epoll.h>

       int epoll_create(int size);	// 要监听的个数
```

返回值：其实是一个文件描述符，句柄（树根）



内核原理：平衡二叉树(红黑树)，返回的描述符指向这根树的树根



- 向之前创建的红黑树添加、修改、或删除节点

```c
       #include <sys/epoll.h>

       int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

           typedef union epoll_data {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;

           struct epoll_event {
               uint32_t     events;      /* Epoll events */
               epoll_data_t data;        /* User data variable */
           };
```

struct epoll_event *event： 结构体指针



op:add/mod/del



- 监听节点

```c
       #include <sys/epoll.h>

       int epoll_wait(int epfd, struct epoll_event *events, // 数组，传出参数
                      int maxevents, int timeout);
```

truct epoll_event *events 返回epoll_ctl中被激活的文件描述符，这是个结构体数组



timeout ：

​				-1 阻塞

​				0 立即返回

​				>0 毫秒

返回值，成功返回有多少个文件描述符就绪。



### 结构体分析

```c
           typedef union epoll_data {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;

           struct epoll_event {
               uint32_t     events;      /* Epoll events */
               epoll_data_t data;        /* User data variable */
           };
```



events:监听事件

data：fd，u32，u64， void * ptr



平时我们都填充fd，用来返回的时候，它就是被激活的文件描述符。

这个epoll_data是我们自己填充的，epoll_wait返回时，不会修改epoll_event中的数据。



---

提升程序执行效率，减少wait执行次数



边沿触发：epoll ET

水平触发：epoll LT



![image-20200605133928894](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200605133928894.png)

当低电平向高电平或高电平到低电平转换时，这种叫边沿触发

当持续高电平或持续低电平时，这种叫小平触发



## epoll ET



EPOLLIN | EPOLLET



ET模式可以减少epoll_wait可以减少调用次数



和read使用非阻塞方式一起使用。



## epoll LT

EPOLLIN 默认情况下就是水平触发



## epoll 非阻塞IO

详见codes/noblock_epolls目录下代码



**其实就是给连接的文件描述符使用fcntl给它设置noblock**



## epoll 反应堆模型(libevent 核心思想实现)







## 线程池



