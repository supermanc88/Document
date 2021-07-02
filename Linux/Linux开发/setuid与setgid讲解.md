# setuid与setgid讲解

文件权限的机制是Linux系统中的一大特色，除了熟知的读(r)、写(w)、执行(x)权限外，还有三个比较特殊的权限，分别为：`setuid`、`setgid`和`stick bit`（粘滞位)



## setuid与setgid

看一下系统中用到它的地方，以/etc/passwd和/usr/bin/passwd为例：

```shell
[root@Salve1 school]# ll /etc/passwd /usr/bin/passwd 
-rw-r--r-- 1 root root 2005 Apr 23 01:25 /etc/passwd 
-rwsr-xr-x 1 root root 23420 Aug 11 2010 /usr/bin/passwd 
```

分析一下，/etc/passwd的权限为 `-rw-r--r--` 也就是说：

该文件的所有者拥有读写的权限，而用户组成员和其它成员只有查看的权限。

我们知道，在系统中我们要修改一个用户的密码，

`root`用户和普通用户均可以用`/usr/bin/passwd someuser`这个命令来修改这个`/etc/passwd`这个文件

`root`用户本身拥有对`/etc/passwd`的写权限，无可厚非；

那普通用户呢，这里就用到了`setuid`，**`setuid`的作用是“让执行该命令的用户以该命令拥有者的权限去执行”**

就是普通用户执行`passwd`时会拥有`root`的权限，这样就可以修改`/etc/passwd`这个文件了。

它的标志为：`s`，会出现在x的地方，例：`-rwsr-xr-x` 。

而`setgid`的意思和它是一样的，即**让执行文件的用户以该文件所属组的权限去执行。**



## stick bit(粘滞位)

看一下系统中用到它的地方，以/tmp为例：

```shell
[root@Salve1 /]# ll -d /tmp 
drwxrwxrwt 13 root root 4096 Apr 23 02:06 /tmp 
```

我们知道/tmp是系统的临时文件目录，所有的用户在该目录下拥有所有的权限，

也就是说在该目录下可以任意创建、修改、删除文件，那如果用户A在该目录下创建了一个文件

如果用户B将该文件删除了，这种情况我们是不能允许的。为了达到该目的，就出现了stick bit（粘滞位）的概念。

它是针对目录来说的，如果该目录设置了stick bit（粘滞位）

则该目录下的文件除了该文件的创建者和root用户可以删除和修改/tmp目录下的stuff

别的用户均不能动别人的，这就是粘滞位的作用。



## 如何设置上述权限

```shell
# 设置setuid权限 
chmod u+s xxx

# 设置setgid权限 
chmod g+s xxx

# 设置stick bit权限，针对目录
chmod o+t xxx

# 设置setuid权限
chmod 4775 xxx

# 设置setgid权限
chmod 2775 xxx

# 设置stick bit权限，针对目录
chmod 1775 xxx
```

> 注意：有时你设置了s或t 权限，你会发现它变成了S或T，
>
> 这是因为在那个位置上你没有给它x（可执行）的权限，这样的话这样的设置是不会有效的，
>
> 你可以先给它赋上x的权限，然后再给s或t 的权限。