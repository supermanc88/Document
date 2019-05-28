在CentOs上安装git

## 查看git服务
先查看git服务，centos上的版本比较低
```
yum info git
```
显示：
```
[root@proud-cubes-3 ~]# yum info git
Loaded plugins: fastestmirror
Loading mirror speeds from cached hostfile
 * base: centos.mirror.lax.us.hostlink.com.au
 * elrepo-kernel: repos.lax-noc.com
 * epel: d2lzkl7pfhq30w.cloudfront.net
 * extras: centos.mirror.lax.us.hostlink.com.au
 * updates: repos-lax.psychz.net
Installed Packages
Name        : git
Arch        : x86_64
Version     : 1.8.3.1
Release     : 20.el7
Size        : 22 M
Repo        : installed
From repo   : updates
Summary     : Fast Version Control System
URL         : http://git-scm.com/
License     : GPLv2
Description : Git is a fast, scalable, distributed revision control system with an
            : unusually rich command set that provides both high-level operations
            : and full access to internals.
            : 
            : The git rpm installs the core tools with minimal dependencies.  To
            : install all git packages, including tools for integrating with other
            : SCMs, install the git-all meta-package.

```

删除掉旧的版本：
```
yum remove git
```
https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.9.5.tar.xz

## 安装git

### 安装信赖

```
yum install autoconf curl-devel expat-devel openssl-devel zlib-devel perl-devel
```

### 下载git最新版本并编译安装
```
[root@tCentos7 ~]# cd /usr/local/src
[root@tCentos7 ~]# wget https://www.kernel.org/pub/software/scm/git/git-2.11.0.tar.gz
[root@tCentos7 ~]# tar -zvxf git-2.11.0.tar.gz
[root@tCentos7 ~]# cd git-2.11.0
[root@tCentos7 ~]# make configure
[root@tCentos7 ~]# ./configure --prefix=/usr/local/git    ### 也可以指定libicon  -with-iconv=/usr/local/libiconv
[root@tCentos7 ~]# make && make install
```
需要等待一段时间

安装完成后，添加到环境变量
```
vim /etc/profile 
// 添加一条
export PATH="/usr/local/git/bin:$PATH"
source /etc/profile   //使配置立即生效
git --version  //查看版本号
```
添加软链接
```
ln -s /usr/local/git/bin/* /usr/bin/
```

## 创建一个git帐户

```
# groupadd git
# useradd git -g git
# passwd git  #参数是用户名
# su - git  //切换git用户
```
> 最好切换到git用户，不然后面新建的git仓库都要改权限

## git手动管理
```
# 进入git账户的主目录
cd ~
# 创建.ssh的配置
mkdir .ssh
# 进入.ssh目录并创建authorize_keys文件，此文件存放客户端远程访问的ssh公钥
cd .ssh
touch authorized_keys
# 设置权限，此步骤不能省略，而且权限也不要改，不然会报错
chmod 700 /home/git/.ssh/
chmod 600 /home/git/.ssh/authorized_keys
```
此时，服务端的配置基本完成，接下来需要把客户端的公钥拷贝到`authorized_keys`文件中。

> authorized_keys文件中的每个ssh的pub都要单独的一行,将密钥上传到.ssh目录
```
cat xxx.pub >> authorized_keys
```
## 服务器创建测试git仓库
在/home/git/目录下创建test.git目录
```
# 切换到git账号
$ su git
# 进入git账号的用户主目录。

$ cd /home/git
# 在用户主目录下创建 test.git仓库的文件夹

$ mkdir test.git  && cd test.git
# 在test.git目录下初始化git仓库

$ git init --bare
# 输出如下内容，表示成功
Initialized empty Git repository in /home/git/test.git/
```

> 当修改了ssh默认的端口时
> git remote add origin ssh://git@xxx:port/repo.git

## 为什么在git服务器上查看不到代码
```
# 在test.git目录下初始化git仓库

$ git init --bare
```
由于上面初始化git仓库的时候有个`--bare`参数，所以这个仓库只存储代码历史提交信息，纯粹是为了共享，不允许用户直接登录服务器上去修改。
