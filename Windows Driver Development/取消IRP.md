# 取消IRP

如果不使用`IoCompleteRequest`函数结束的话，也可以使用`IoSetCancelRoutine`结束，也就是取消IRP请求。

```
PDRIVER_CANCEL IoSetCancelRoutine(
  PIRP           Irp,
  PDRIVER_CANCEL CancelRoutine
);
```
`CancelRoutine`这个是取消函数的函数指针。

IoSetCancelRoutine可以将一个取消函数与该IRP关联，一旦取消IRP请求的时候，这个取消函数会被执行。IoSetCancelRoutine函数也可以用来删除取消函数，当输入的Cancelroutine参数为NULL时，则删除原来的取消函数。
