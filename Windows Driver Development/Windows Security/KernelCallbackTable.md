## 怎样获取KernelCallbackTable


首先KernelCallbackTable是存放在`PEB`中，需要找一个普通的进程进行调试：

![image-20200929162534322](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200929162534322.png)

查看`PEB`及其结构体：

![image-20200929162706499](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200929162706499.png)





![image-20200929162900653](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200929162900653.png)

查看`KernelCallbackTable`中的内容：

![image-20200929162944726](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200929163010604.png)

Windbg 命令 `dqs`： 会显示给定范围的内存内容。当该内存是符号表中的一系列地址时，相应的符号也会显示出来。

![image-20200929163010604](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200929162944726.png)