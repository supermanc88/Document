# UDP协议



使用场景：对实时性要求较高，视频会议、视频电话、广播、飞秋



QQ：TCP+UDP



但为什么没有发现QQ没有丢包，是因为 UDP + 应用层自定义协议弥补UDP的丢包



使用 `recvfrom`和`sendto`两个函数



也不需要多线程、多进程



默认支持并发



## 广播

IP：192.168.1.255(广播地址)

IP：192.168.1.1(网关)



使用`setsockopt`给socket赋予广播权限。



## 多播(组播)

组播是另外的一个地址段，不是广播地址。



server端使用`setsockopt`给socket赋予组播权限。

client端使用`setsockopt`使socket加入组播。