# StartIO例程

StartIO例程能够保证各个并行的IRP顺序执行，即串行化。

## 并行执行与串行执行
在很多情况时，对设备的操作必须是串行执行，而不能并行执行。例如，对串口的操作，假如有N个线程同时操作串口设备时，必须将这些操作排队，然后一一进行处理。如果不做串行化处理，当一个操作没有完毕时，新的操作又开始，这必然会导致操作的混乱。

当一个新的IRP请求来临时，首先检查设备是否处于“忙”状态。设备在初始化的时候为“空闲”状态。当设备处在“空闲”的时候，可以处理一个IRP请求，并改变当前设备状态为“忙”状态。如果设备处于“忙”状态，则将新来的IRP插入队列，并立刻返回，IRP留在以后处理。

当设备状态由“忙”转入“空闲”状态时，则从队列取出一个IRP进行处理，并重新将状态变为“忙”。这样，周而复始地将所有IRP的请求串行都处理了。

## StartIO例程

操作系统提供了一个IRP队列来实现串行,这个队列的队列头保存在设备对象的`DeviceObject->DeviceQueue`子域中。**插入和删除队列中的元素都是操作系统负责的。在使用这个队列的时候，需要向系统提供一个叫做StartIo的例程**。

```
DriverObject->DriverStartIO = HelloDDKStartIO;
```

**注意StartIO是执行在`DISPATCH_LEVEL`级别上，因此声明如下：**

```
#pragma LOCKEDCODE
VOID HelloDDKStartIO(
IN PDEVICE_OBJECT DeviceObject,
IN PRIP Irp)
{
...
}
```
