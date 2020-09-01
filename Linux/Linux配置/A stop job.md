**一、现象**

Linux关机时提示A stop job is running for ..

导致关机慢。

**二、解决方法**

编辑:

```sh
/etc/systemd/system.conf
```

修改下面两个变量为：

```sh
DefaultTimeoutStartSec=10s
DefaultTimeoutStopSec=10s
```

执行：

```sh
systemctl daemon-reload
```