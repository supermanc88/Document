## 工具

1. windows server 2003 sp2 （sp1也可以，或xp x64）
2. Vmware


## 编译WRK源码

将wrk-v1.2拷贝到虚拟机server 2003中，添加环境变量`Path`，内容为`path\tools\x86`


cmd命令cd到`path\base\ntos`目录下，执行命令`nmake -nologo x86=`

注：

1. 可能提示系统缺少`msvcr71.dll`和`msvcp71.dll`，从网上找个加到`system32`目录下即可。
2. 源码路径中最好没有中文和空格

编译出：

- wrkx86.exe

## 选择Hal

WRK提供了多个版本的hal，为了知道当前系统的hal类型：

```
link -dump -all \WINDOWS\system32\hal.dll | findstr pdb
```

![image](https://raw.githubusercontent.com/supermanc88/Document/master/VimWindows%20Security/images/1572507617(1).jpg)

在WRK中，根据以下关系选择相应的HAL库：

halacpi.dll -> halacpim.dll

halaacpi.dll->halmacpi.dll

halapic.dll->halmps.dll

所以应将`halmacpi.dll`拷贝到`system32`目录下。


## 环境搭建

1. 将`wrkx86.exe`复制到`system32`下
2. 将对应的`hal.dll`复制到`system32`下
3. 显示隐藏文件，在`boot.ini`中添加以下：

```
multi(0)disk(0)rdisk(0)partition(1)\WINDOWS="WRK V1.2" /kernel=wrkx86.exe /hal=halmacpi.dll
multi(0)disk(0)rdisk(0)partition(1)\WINDOWS="WRK V1.2[debug]" /kernel=wrkx86.exe /hal=halmacpi.dll /debug /debugport=com1 /baudrate=115200
```

至此，调试环境搭建完成