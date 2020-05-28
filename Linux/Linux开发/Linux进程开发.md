# Linux进程

## 进程的基本状态

有五种：

1. 初始态
2. 就绪态
3. 运行态
4. 挂起态
5. 终止态



其中初始态与就绪态常结合来看。

![image-20200528151830099](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200528151830099.png)



## 环境变量

环境变量以进程为单位，每个进程的环境变量可能都不一样。



```c
extern char ** environ;
```





## fork 函数

返回值：有两个返回值

1. 返回子进程的pid(非负整数)
2. 返回0

![image-20200528164143353](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200528164143353.png)

如上图所示，当fork执行后，会有子进程出现，fork函数之上的代码在子进程中不再运行，现在子进程中也有一个fork函数，返回值是什么呢？

父进程返回子进程PID

子进程返回0表示fork成功执行



在之后的代码中，可根据fork的返回值，用来判断是在父进程中还是在子进程中。



![image-20200528165826971](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200528165826971.png)



## 父子进程共享

父子进程在fork之后，有哪些是相同的，哪些是不同的？

相同处：全局变量、.data、.text、栈、堆、环境变量、用户ID、宿主目录、进程工作目录、信号处理方式。。。（但不可共享，是独立的进程）

不同处：进程ID、fork返回值、进程运行时间、父进程ID、闹钟（定时器）、未决信号集



父子进程间遵循**读时共享写时复制（共享同一块物理地址）**的原则。这样设计，无论子进程执行父进程的逻辑还是执行自己的逻辑都能节省内存开销。

父子进程共享：

1. 文件描述符（打开文件的结构体）
2. mmap建立的映射区（进程间通信）