# Linux代理

在`/etc/profile.d`文件夹下创建文件`proxy.sh`



添加如下内容：

```shell
export proxy="http://192.168.45.1:7890"
export http_proxy=$proxy
export https_proxy=$proxy
export ftp_proxy=$proxy
export no_proxy="localhost, 127.0.0.1, ::1"
```



reboot或 `source /etc/profile.d/proxy.sh`生效



