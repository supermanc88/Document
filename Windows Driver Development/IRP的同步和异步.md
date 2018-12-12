## 同步和异步区别
- 同步
![image](./images/1544435336(1).jpg)

如上图，当程序调用DeviceIocontrol时，它的内部会创建一个`IRP_MJ_DEVICE_CONTROL`的IRP，这个IRP传送到驱动的派遣函数中，处理该IRP需要一段时间，直到该IRP处理完毕后，DeviceIoControl才会返回。

- 异步
![image](./images/1544436001(1).jpg)

和同步不同的是，DeviceIoControl发送IRP到驱动后，不会等待该IRP结束，而是直接返回。当IRP经过一段时间被结束时，操作系统会触发一个IRP相关事件，这个事件可以通知应用程序IRP请求被执行完毕。


## IRP的同步完成与异步完成

有两种处理IRP请求方式：
- 同步：在派遣函数中直接结束IRP请求
- 异步：在派遣函数中不结束IRP请求，而是让派遣函数直接返回。IRP在以后的某个时候再进行外理。

### ReadFile的同步和异步
- ReadFile的最后一个参数overlap，如果设置的话，会以异步读取
- ReadFileEx异步

### IRP的异步完成

IRP的异步完成就是不在派遣函数中调用IoCompleteRequest函数。调用该函数意味着IRP请求的结束，也标志着本次对设备操作的结束。

- 首先使用`IoMarkIrpPending`函数将IRP设置为挂起。
- 返回`STATUS_PENDING`状态
- 保存这个IRP，因为要在别的时候结束它
- 在其他地方处理这个IRP


