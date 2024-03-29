# Linux文件描述符表

对于内核而言，所有打开的文件都通过`文件描述符`引用。文件描述符是一个非负整数。当打开一个现有文件或创建一个新文件时，内核向进程返回一个文件描述符。



> Unix系统shell把文件描述符0与进程的标准输入关联，文件描述符1与标准输出关联，文件描述符2与标准错误关联。

STDIN_FILENO/STDOUT_FILENO/STDERR_FILENO分别代表文件描述符0、1、2。



文件描述符的变化范围是0-OPEN_MAX-1。也就是说允许每个进程最多打开这么多个文件。



不用管这个文件描述符用0、1、2、3。。。这些数字很简单，其实这些数字在内部都对应着不同的指针，这些指针又指向一个文件结构或一个文件节点。



## /dev/fd/

在/dev/fd目录下，其目录项是名为0、1、2等的文件。打开文件/dev/fd/n等效于复制描述符n



如：

```c
fd = open("/dev/fd/0", mode);

// 等同

fd = dup(0);
```

一般情况下，参数mode在大部分系统下被忽略，而另外一些系统要求mode必须是所引用的文件**初始打开时所使用的打开模式的一个子集**。



![image-20200606172438112](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200606172438112.png)