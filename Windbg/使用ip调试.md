# 使用IP方式进行Windbg调试

首先要确认两台计算机可以互相ping通

假设主机ip：192.168.137.1

被调试机ip：192.168.137.12

## 主机
运行cmd
```
windbg -k net:port=50000,key=1.2.3.4
```
> 此时windbg会弹出防火墙提示，全部允许
## 被调试机

以管理员身份打开cmd
```
# 启动调试
bcdedit /set {default} debug yes
# 启用测试签名
bcdedit /set testsigning on
# 设置主机的ip地址
bcdedit /dbgsettings net hostip:192.168.137.1 port:50000 key:1.2.3.4
# 确认dbgsettings配置正确
bcdedit /dbgsettings
```

> 在配置dbgsettings的时候可以不用设置key，系统会自动生成，但要保证两台机器的key的一致

