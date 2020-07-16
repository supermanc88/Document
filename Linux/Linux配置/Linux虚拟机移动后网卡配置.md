# 解决centos网卡启动device eth0 does not seem to be present, delaying initialization



启动系统后不能上网，通过ifconfig查看网卡没有eth0，重启网卡服务，但是显示出错，第三行显示：device eth0 does not seem to be present, delaying initialization.  然后想到是不是ifcfg-eth0的配置文件里保存了以前的MAC地址，就把这一行删除掉在重启网卡，还是一样的错误，随后网上查了下资料，把/etc/udev/rules.d/70-persistent-net.rules 删除后重启机器就可以了，因为这个文件绑定了网卡和mac地址，所以换了网卡以后MAC地址变了，所以不能正常启动，也可以直接编辑这个配置文件把里面的网卡和mac地址修改乘对应的，不过这样多麻烦，直接删除重启，它会自动生成个。



![wKiom1T-XYbi-EcsAAB9gymKFEg125](https://raw.githubusercontent.com/supermanc88/ImageSources/master/wKiom1T-XYbi-EcsAAB9gymKFEg125.jpg)



```shell
vi /etc/sysconfig/network-scripts/ifcfg-eth0

rm /etc/udev/rules.d/70-persistent-net.rules 
```



