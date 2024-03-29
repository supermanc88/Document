# 内核模块的调试方式



## 获取驱动模块的基址

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/33695fd212954f2aa4dcf513ac40d5f7.png)

```shell
cat /proc/modules | grep drv1


cat /sys/module/drv1/section/.text
```



两种方式均可查到加载基地址。



## 设置调试模块



加载符号文件同时为驱动的Open函数设置断点，如下图所示

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/57df302f6c58477bbc2854474bb94817.png)



需要注意的是不管是图中的驱动文件还是驱动程序源文件都应该是位于主机中（要把被调试机编译好的驱动文件及源文件，拷贝一份给调试机），需要注意的是调试机中拷贝过来的驱动文件可以放置在任何位置，而源文件的位置应该和被调试机中源文件位置保持一致，这样才能进行源码级调试（上面黄框中源代码的位置表示是在被调试机中的位置，所以调试机驱动的源码也应该放在这样的位置，这样驱动调试器才能找到符号位置。如果对驱动程序不进行源码调试，只进行汇编调试就不需要源文件），而加载地址是被调试机的驱动加载基地址，这一点千万别搞错了。



**一句话就是，源文件路径一致，省事**