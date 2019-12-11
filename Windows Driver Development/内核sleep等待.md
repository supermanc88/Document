

```C
NTSTATUS KeDelayExecutionThread(
  KPROCESSOR_MODE WaitMode,
  BOOLEAN         Alertable,
  PLARGE_INTEGER  Interval
);
```
参数`Interval`：

指定等待发生的绝对或相对时间(以100纳秒为单位)。负值表示相对时间。绝对过期时间跟踪系统时间的任何变化;相对过期时间不受系统时间更改的影响。

```
1秒 = 1000毫秒 = 1000000000纳秒

1毫秒 = 1000000纳秒
```

常用转换函数`RtlConvertLongToLargeInteger`将long转为largeinteger

