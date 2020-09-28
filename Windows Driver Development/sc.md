# sc 服务管理

## 安装驱动

```shell
sc create SERVICE_NAME binPath= "SERVICE_PATH" type= kernel start= auto
```

**注：binpath、type、start等号后面的空格，一定要有**



## 启动驱动

```shell
sc start SERVICE_NAME
```





## 停止驱动

```shell
sc stop SERVICE_NAME
```





## 获取驱动状态

```shell
sc query SERVICE_NAME
```





## 卸载驱动

```shell
sc delete SERVICE_NAME
```



