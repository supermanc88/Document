# Linux程序版本管理

使用`update-alternatives`， 其实就是一个符号链接管理工具

```shell
NAME
       alternatives - maintain symbolic links determining default commands

SYNOPSIS
       alternatives  [options]  --install  link  name  path priority [--slave link name path]...
       [--initscript service]

       alternatives [options] --remove name path

       alternatives [options] --set name path

       alternatives [options] --auto name

       alternatives [options] --display name

       alternatives [options] --config name

```

例子：

```shell
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50 --slave /usr/bin/g++ g++ /usr/bin/g++-5

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 70 --slave /usr/bin/g++ g++ /usr/bin/g++-7
```



选择要使用的版本：

```shell
[root@localhost ~]# update-alternatives --config gcc

There are 2 programs which provide 'gcc'.

  Selection    Command
-----------------------------------------------
   1           /usr/bin/gcc-4.1.2
*+ 2           /usr/local/bin/gcc

Enter to keep the current selection[+], or type selection number: 
```



删除：

```shell
update-alternatives --remove gcc /usr/bin/gcc-4.1.2
```

