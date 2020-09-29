由于在xp下，virtualkd不能使用，所以不借助virtualkd

## 虚拟机配置

![vm](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1559628616(1).jpg)


## 在xp下运行msconfig

![xp](https://raw.githubusercontent.com/supermanc88/ImageSources/master/1559628917(1).jpg)
打开调试

## windbg配置：

windbg启动参数添加
```
 -b -k com:pipe,port=\\.\pipe\com_1,resets=0
```