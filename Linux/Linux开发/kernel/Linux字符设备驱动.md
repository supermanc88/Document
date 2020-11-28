## 怎样获取字符设备动态申请的设备号


```shell
[root@localhost module_cdev]# cat /proc/devices | grep "module"
247 module_cdev
```


## 生成一个设备节点与设备关联

```shell
[root@localhost module_cdev]# cat /proc/devices | grep "module"
247 module_cdev
[root@localhost module_cdev]# man mknod
Formatting page, please wait...
[root@localhost module_cdev]# 
[root@localhost module_cdev]# mknod cdev_node c 247 0
[root@localhost module_cdev]# ls
cdev_node  module_cdev.c            module_cdev.mod.c  modules.order
Makefile   module_cdev.ko           module_cdev.mod.o  Module.symvers
Makefile~  module_cdev.ko.unsigned  module_cdev.o
```

