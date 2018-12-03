# 分层驱动IRP探究

I/O管理器会创建一个IRP堆栈，堆栈中的元素分别对应于分层驱动中的设备对象。每个设备拥有IRP堆栈中的一个位置，可以使用`IoGetCurrentIrpStackLocation`获取当前IRP信息。

例子是一个TDI驱动，驱动程序创建一个设备对象attach到Tcp设备对象上。

设备堆栈如下：
```
kd> !devstack 0xfffffa80`058b1b70
  !DevObj           !DrvObj            !DevExt           ObjectName
> fffffa80058b1b70  \Driver\TdiDriverTest00000000  
  fffffa80044fd060  \Driver\tdx        fffffa80044fd1b0  Tcp
```
看一看`IoSkipCurrentIrpStackLocation(Irp)`和`IoCallDriver(OldObj, Irp)`究竟做了什么？

查看一下IRP：
```
kd> !irp 0xfffffa80`06cf8e10
Irp is active with 2 stacks 2 is current (= 0xfffffa8006cf8f28)
 No Mdl: No System Buffer: Thread fffffa8003d27040:  Irp stack trace.  
     cmd  flg cl Device   File     Completion-Context
 [N/A(0), N/A(0)]
            0  0 00000000 00000000 00000000-00000000    

			Args: 00000000 00000000 00000000 00000000
>[IRP_MJ_INTERNAL_DEVICE_CONTROL(f), N/A(2)]
            0 e0 fffffa80058b1b70 fffffa80055f6dd0 fffff88004cf6a90-00000000 Success Error Cancel 
	       \Driver\TdiDriverTest	afd!AfdRestartDeviceControl
			Args: 00000000 00000000 0x3 00000000
```
发现`fffffa80058b1b70`正好是驱动程序TdiDriverTest创建的设备对象。当前IRP给这个设备对象发送了`IRP_MJ_INTERNAL_DEVICE_CONTROL`指令。

当执行一次`IoSkipCurrentIrpStackLocation(Irp)`后，再次查看：

发现`>`已经不在，

调用IoCallDriver后，传递给IoCallDriver的IRP指针不再有效，无法安全地解除引用。