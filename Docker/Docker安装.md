# Docker安装



## 环境

Debian 9



## 官方安装脚本

```shell
curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun
```



## 安装 Docker Engine-Community

在首次安装Docker Engine-community之前，需要设置Docker仓库(也就是源地址？)。



### 设置仓库

安装 apt 依赖包，用于通过 HTTPS 来获取仓库。

```shell
apt-get update

apt-get install \
                         apt-transport-https \
                         ca-certificates \
                         curl \
                         gnupg2 \
                         software-properties-common

```



添加Docker的官方GPG密钥：

```shell
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo apt-key add -
```



查看是否正确安装上GPG密钥：

```shell
root@debian /h/superman# apt-key finger
/etc/apt/trusted.gpg
--------------------
pub   rsa4096 2017-02-22 [SCEA]
      9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88
uid           [ unknown] Docker Release (CE deb) <docker@docker.com>
sub   rsa4096 2017-02-22 [S]

```



**设置稳定版仓库：**

```shell
add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/debian \
  $(lsb_release -cs) \
  stable"
```



## 安装Docker Engine-Community

```shell
apt-get update
apt-get install docker-ce docker-ce-cli containerd.io
```



## 验证是否正确安装

```shell
root@debian:/home/superman# docker run hello-world
Unable to find image 'hello-world:latest' locally
latest: Pulling from library/hello-world
0e03bdcc26d7: Pull complete 
Digest: sha256:d58e752213a51785838f9eed2b7a498ffa1cb3aa7f946dda11af39286c3db9a9
Status: Downloaded newer image for hello-world:latest

Hello from Docker!
This message shows that your installation appears to be working correctly.

To generate this message, Docker took the following steps:
 1. The Docker client contacted the Docker daemon.
 2. The Docker daemon pulled the "hello-world" image from the Docker Hub.
    (amd64)
 3. The Docker daemon created a new container from that image which runs the
    executable that produces the output you are currently reading.
 4. The Docker daemon streamed that output to the Docker client, which sent it
    to your terminal.

To try something more ambitious, you can run an Ubuntu container with:
 $ docker run -it ubuntu bash

Share images, automate workflows, and more with a free Docker ID:
 https://hub.docker.com/

For more examples and ideas, visit:
 https://docs.docker.com/get-started/

```

