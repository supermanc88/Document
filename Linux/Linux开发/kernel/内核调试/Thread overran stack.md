# Thread overran stack, or stack corrupted

驱动开发遇到如下问题：



![image-20201210114625948](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210114625948.png)



## 查看用户空间栈大小

```shell
ulimit -s			// 这里显示的单位是kb
```

![image-20201210115543494](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210115543494.png)

所以实验机默认栈大小是8M大小



```shell
ulimit -a
```

![image-20201210115720611](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210115720611.png)





## 修改用户空间栈大小

临时修改：

```shell
ulimit -s 102400				// 临时修改为100M
```



永久修改：

```
方法一：可以在/etc/rc.local 内加入 ulimit -s 102400 则可以开机就设置栈空间大小，任何用户启动的时候都会调用。

方法二：修改配置文件/etc/security/limits.conf
```



## 内核栈大小

```c
#define THREAD_SIZE_ORDER	1
#define THREAD_SIZE		(PAGE_SIZE << THREAD_SIZE_ORDER)
```

内核栈大小默认是8K，如果要修改的话，只能重新编译内核。



## crash定位问题

```shell
task
```



![image-20201210145833637](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210145833637.png)



```shell
thread_info 0xffff8800b91e4000
```





![image-20201210145615228](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210145615228.png)

这些数据一看你很离谱了，cpu这么大的数，另外thread_info中的task的值上面的也不一样。





相关链接：

[内核栈溢出 | Linux Performance](http://linuxperf.com/?p=116)

