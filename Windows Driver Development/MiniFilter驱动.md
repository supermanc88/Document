## MiniFilter驱动加载顺序

每个minifilter驱动程序必须具有称为`altitude`的唯一标识符。
当加载微过滤器驱动程序时，微过滤器驱动程序的高度定义其相对于I/O堆栈中其他微过滤器驱动程序的位置。高度是一个无限精度的字符串，被解释为十进制数字。具有较低数值高度的微过滤器驱动器被加载到具有较高数值的微过滤器驱动器下方的I/O堆栈中。

高度越低，加载越早。

下图显示了带有过滤器管理器和三个微过滤器驱动程序的简化I/O堆栈：

![image](./images/1548818559(1).jpg)

微过滤器驱动程序可以过滤基于IRP的I/O操作以及快速I/O和文件系统过滤器回调操作。对于它选择过滤的每个I/O操作，minifilter驱动程序可以注册操作前回调例程，后操作回调例程或两者。

假设上图中的所有三个微过滤器驱动程序都注册了相同的I/O操作，过滤器管理器将按照从高到低(A,B,C)的高度顺序调用其操作前回调例程，然后转发I/O请求下一个较低的驱动程序进行进一步处理。当过滤器管理器收到完成的I/O请求时，它会以相反的顺序从最低到最高(C,B,A)调用每个minifilter驱动程序的后操作回调例程。

## Minifilter的启动和停止

在系统运行时，可以通过服务启动请求：

```
sc start

net start
```
或通过显式加载请求
```
fltmc load
```

加载minifilter驱动程序时会调用minifilter驱动程序的DriverEntry例程，因此minifilter驱动程序可以执行初始化，该初始化将应用于minifilter驱动程序的所有实例。在其DriverEntry例程中，minifilter驱动程序调用`FltRegisterFilter`向过滤管理器注册回调例程，和`FltStartFilter`通知过滤管理器minifilter驱动程序已准备好开始附加到卷并过滤I/O请求。


在挂载卷后的第一个创建操作上调用其`InstanceSetupCallBack`例程，自动通知微过滤器驱动程序有关可用卷的信息。

```
fltmc detach
```
停止过程如下：

- 如果minifilter驱动程序注册了`InstanceTeardownStartCallback`回调例程，则过滤管理器会在卸载过程开始时调用它
- 在卸载期间，任何当前正在执行的操作前或操作后回调例程继续正常处理，等待后操作回调的任何I/O请求可以被排空或取消，并且由微过滤器驱动程序生成的任何I/O请求继续正常处理直到他们完成。
- 如果minifilter驱动程序注册了`InstanceTeardownCompleteCallback`例程，则过滤管理器会在完成所有未完成的I/O操作后调用此例程。在此例程中，minifilter驱动程序将关闭所有仍处于打开状态的文件。
- 释放所有对实例的未完成引用后，过滤管理器将删除剩余的上下文，并且实例将完全拆除。


在系统运行时，可以通过服务停止请求：
```
sc stop
net stop
```

或
通过显式卸载请求
```
fltmc unload
```
卸载minifilter请求。

卸载minifilter驱动程序时会调用minifilter驱动程序的`FilterUnloadCallback`例程。此例程关闭所有打开的通信服务器端口，调用`FltUnregisterFilter`，并执行任何所需的清理。注册此例程是可选的，但是，如果minifilter驱动程序未注册此例程，则无法卸载minifilter驱动。

