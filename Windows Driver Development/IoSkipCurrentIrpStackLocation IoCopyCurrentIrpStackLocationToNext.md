# IoSkipCurrentIrpStackLocation IoCopyCurrentIrpStackLocationToNext 两个函数的区别

这两个函数都经常和 `IoCallDriver`函数配合使用


看一下`IoCallDriver`的源码：

```C
NTSTATUS
FASTCALL
IofCallDriver(IN PDEVICE_OBJECT DeviceObject,
              IN PIRP Irp)
{
    PDRIVER_OBJECT DriverObject;
    PIO_STACK_LOCATION StackPtr;

    /* Make sure this is a valid IRP */
    ASSERT(Irp->Type == IO_TYPE_IRP);

    /* Get the Driver Object */
    DriverObject = DeviceObject->DriverObject;

    /* Decrease the current location and check if */
    Irp->CurrentLocation--;
    if (Irp->CurrentLocation <= 0)
    {
        /* This IRP ran out of stack, bugcheck */
        KeBugCheckEx(NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR)Irp, 0, 0, 0);
    }

    /* Now update the stack location */
    StackPtr = IoGetNextIrpStackLocation(Irp);
    Irp->Tail.Overlay.CurrentStackLocation = StackPtr;

    /* Get the Device Object */
    StackPtr->DeviceObject = DeviceObject;

    /* Call it */
    return DriverObject->MajorFunction[StackPtr->MajorFunction](DeviceObject,
                                                                Irp);
}
```

其中的`CurrentLocation`和`CurrentStackLocation`都进行的自减


## IoSkipCurrentIrpStackLocation

```C
FORCEINLINE
VOID
IoSkipCurrentIrpStackLocation(
  _Inout_ PIRP Irp)
{
  ASSERT(Irp->CurrentLocation <= Irp->StackCount);
  Irp->CurrentLocation++;
#ifdef NONAMELESSUNION
  Irp->Tail.Overlay.s.u.CurrentStackLocation++;
#else
  Irp->Tail.Overlay.CurrentStackLocation++;
#endif
}
```

此时调用`IoCallDriver`函数时，相当于下层的驱动设备还是使用的当前的IRPStackLocation内容


## IoCopyCurrentIrpStackLocationToNext

```C
FORCEINLINE
VOID
IoCopyCurrentIrpStackLocationToNext(
    _Inout_ PIRP Irp)
{
    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextIrpSp;
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);
    RtlCopyMemory(nextIrpSp, irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine));
    nextIrpSp->Control = 0;
}
```

**注意：此函数拷贝的时候，只拷贝到了`CompletionRoutine`之前，下层的驱动设备使用的其实和当前的IRP是一样的，这样拷贝的作用是，当为当前设备设置完成函数时，不会影响到下层设备也被设置完成函数，如果将整个IRPStackLocation拷贝到下一层，那么下一层也会被设置完成函数，下层的完成函数可能会影响到IRP的完成。**


关于IrpStackLocation的讲解参阅 [分层驱动.md](./分层驱动.md)