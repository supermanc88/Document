## MiniFilter驱动加载顺序

每个minifilter驱动程序必须具有称为`altitude`的唯一标识符。
此值在驱动的inf文件中定义，例如：Instance1.Altitude      = "370030"

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

**MiniFilter注册相关代码实现**
```
// 此段代码应该放在驱动初始化DriverEntry处。
status = FltRegisterFilter(DriverObject, &FilterRegistration, &g_FilterHandle);		//注册过滤器

if (NT_SUCCESS(status))
{
	status = FltStartFiltering(g_FilterHandle);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("MiniFilter注册失败\n"));
		FltUnregisterFilter(g_FilterHandle);
	}
}

```

```
// minifilter反注册放在minifilter卸载回调函数中
NTSTATUS
	MiniFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
	)
{
	//卸载回调函数
	FltUnregisterFilter(g_FilterHandle);
	return STATUS_SUCCESS;
}
```


### FltRegisterFilter函数注册了什么？

以下是API的声明：

```
NTSTATUS FLTAPI FltRegisterFilter(
  PDRIVER_OBJECT         Driver,
  const FLT_REGISTRATION *Registration,
  PFLT_FILTER            *RetFilter
);
```

注意第二个参数`FLT_REGISTERATION`结构体。

```
typedef struct _FLT_REGISTRATION {
  USHORT                                      Size;
  USHORT                                      Version;
  FLT_REGISTRATION_FLAGS                      Flags;
  const FLT_CONTEXT_REGISTRATION              *ContextRegistration;
  const FLT_OPERATION_REGISTRATION            *OperationRegistration;
  PFLT_FILTER_UNLOAD_CALLBACK                 FilterUnloadCallback;
  PFLT_INSTANCE_SETUP_CALLBACK                InstanceSetupCallback;
  PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK       InstanceQueryTeardownCallback;
  PFLT_INSTANCE_TEARDOWN_CALLBACK             InstanceTeardownStartCallback;
  PFLT_INSTANCE_TEARDOWN_CALLBACK             InstanceTeardownCompleteCallback;
  PFLT_GENERATE_FILE_NAME                     GenerateFileNameCallback;
  PFLT_NORMALIZE_NAME_COMPONENT               NormalizeNameComponentCallback;
  PFLT_NORMALIZE_CONTEXT_CLEANUP              NormalizeContextCleanupCallback;
  PFLT_TRANSACTION_NOTIFICATION_CALLBACK      TransactionNotificationCallback;
  PFLT_NORMALIZE_NAME_COMPONENT_EX            NormalizeNameComponentExCallback;
  PFLT_SECTION_CONFLICT_NOTIFICATION_CALLBACK SectionNotificationCallback;
} FLT_REGISTRATION, *PFLT_REGISTRATION;
```

此结构体包含了minifilter所用到的回调函数，除了`size`和`version`参数不为空且固定以外，其余参数皆可为NULL。

- size处固定填`sizeof(FLT_REGISTRATION)`
- version处固定填`FLT_REGISTRATION_VERSION`
- Flags处一般为NULL
- ContextRegistration处是一个FLT_CONTEXT_REGISTRATION结构的变长数组，用于标明minifilter使用的上下文类型。数组必须以`{FLT_CONTEXT_END}`结尾,在分配上下文时会使用到。示例如下：
	```
	CONST FLT_CONTEXT_REGISTRATION ContextNotifications[] = {

		{ FLT_VOLUME_CONTEXT,
		0,
		CleanupVolumeContext,
		sizeof(VOLUME_CONTEXT),
		FILE_DISK_POOL_TAG },

		{ FLT_STREAMHANDLE_CONTEXT,
		0,
		NULL,
		sizeof(SCANNER_STREAM_HANDLE_CONTEXT),
		FILE_DISK_POOL_TAG },

		{ FLT_CONTEXT_END }
	};
	```
- OperationRegistration是一个`FLT_OPERATION_REGISTRATION`结构的变长数组，每个IRP请求类型对应一个，minifilter为它注册预操作和后操作。数组中的最后一个元素必须以`{IRP_MJ_OPERATION_END}`结尾，示例如下：
	```
	CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

	#if 0 // TODO - List all of the requests to filter.
		{ IRP_MJ_CREATE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_CREATE_NAMED_PIPE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_CLOSE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_READ,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_WRITE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_QUERY_INFORMATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SET_INFORMATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_QUERY_EA,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SET_EA,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_FLUSH_BUFFERS,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_QUERY_VOLUME_INFORMATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SET_VOLUME_INFORMATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_DIRECTORY_CONTROL,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_FILE_SYSTEM_CONTROL,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_DEVICE_CONTROL,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_INTERNAL_DEVICE_CONTROL,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SHUTDOWN,
		  0,
		  FsFilter1PreOperationNoPostOperation,
		  NULL },                               //post operations not supported

		{ IRP_MJ_LOCK_CONTROL,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_CLEANUP,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_CREATE_MAILSLOT,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_QUERY_SECURITY,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SET_SECURITY,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_QUERY_QUOTA,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_SET_QUOTA,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_PNP,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_RELEASE_FOR_MOD_WRITE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_RELEASE_FOR_CC_FLUSH,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_NETWORK_QUERY_OPEN,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_MDL_READ,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_MDL_READ_COMPLETE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_PREPARE_MDL_WRITE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_MDL_WRITE_COMPLETE,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_VOLUME_MOUNT,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

		{ IRP_MJ_VOLUME_DISMOUNT,
		  0,
		  FsFilter1PreOperation,
		  FsFilter1PostOperation },

	#endif // TODO

		{ IRP_MJ_OPERATION_END }
	};
	```
- FilterUnloadCallback注册过滤器的卸载例程，如果为NULL的话，则minifilter不能卸载。
- InstanceSetupCallback是minifilter安装的回调例程，每个卷要加载都会经过此例程的处理。
- 其余参数用的不多，自己也不太了解


## MiniFilter通信

minifilter通过`通信端口`进行用户模式和内核模式之间的通信。

### 内核模式

- FltCloseClientPort
- FltCloseCommunicationPort
- FltCreateCommunicationPort
- FltSendMessage

#### 创建通信端口

拿下面代码作讲解：

```
	status = RtlCreateSecurityDescriptor(&miniFltSd, SECURITY_DESCRIPTOR_REVISION);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	RtlSetDaclSecurityDescriptor(&miniFltSd, TRUE, NULL, FALSE);

	RtlInitUnicodeString(&uniString, MINISPY_PORT_NAME);

	InitializeObjectAttributes(&miniFltOa,
		&uniString,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		&miniFltSd);



	status = FltCreateCommunicationPort(g_FilterHandle,
		&g_ServerPort,
		&miniFltOa,
		NULL,
		FDMiniConnect,
		FDMiniDisconnect,
		FDMiniMessage,
		50);			//修改客户端最大连接数

	if (!NT_SUCCESS(status)) {

		if (NULL != g_ServerPort) {
			FltCloseCommunicationPort(g_ServerPort);
		}

		if (NULL != g_FilterHandle) {
			FltUnregisterFilter(g_FilterHandle);
		}
	}
```
这一套就是一个固定的流程，着重看一下`FltCreateCommunicationPort`这个函数

```
NTSTATUS FLTAPI FltCreateCommunicationPort(
  PFLT_FILTER            Filter,
  PFLT_PORT              *ServerPort,
  POBJECT_ATTRIBUTES     ObjectAttributes,
  PVOID                  ServerPortCookie,
  PFLT_CONNECT_NOTIFY    ConnectNotifyCallback,
  PFLT_DISCONNECT_NOTIFY DisconnectNotifyCallback,
  PFLT_MESSAGE_NOTIFY    MessageNotifyCallback,
  LONG                   MaxConnections
);
```
- Filter 这里填`FltRegisterFilter`返回的过滤器的句柄
- ServerPort 这里生成一个用于通信的端口
- ObjectAttributes 指向`OBJECT_ATTRIBUTES`结构的指针
- ConnectNotifyCallback 当用户模式应用程序向驱动发送连接请求时，minifilter就会调用此例程
- DisconnectNotifyCallback 当用户态与内核态连接结束时，minifilter会调用此例程
- MessageNotifyCallback 当用户态与内核态传送数据时，用户模式程序调用`FilterSendMessage`通过客户端端口向minifilter驱动程序发送消息，minifilter会调用此例程
- MaxConnections 此端口允许的最大并发客户端连接数，此参数必需，且必须大于0

详细了解一下接收消息，该函数定义如下：
```
typedef NTSTATUS
(*PFLT_MESSAGE_NOTIFY) (
      IN PVOID PortCookie,
      IN PVOID InputBuffer OPTIONAL,
      IN ULONG InputBufferLength,
      OUT PVOID OutputBuffer OPTIONAL,
      IN ULONG OutputBufferLength,
      OUT PULONG ReturnOutputBufferLength
      );
```
- InputBuffer 是接收到的缓冲区
- InputBufferLength 接收到的缓冲区长度
- OutputBuffer 如果minifilter需要回复数据，**注意，OutputBuffer是指向用户模式缓冲区的指针，此指针仅在用户模式过程的上下文中有效，并且只能从try/except块中访问。minifilter调用`ProbeForWrite`来验证这个指针，但是他不能确保缓冲区正确对齐。如果缓冲区包含具有对齐要求的结构，minifilter驱动程序负责执行任何必要的对齐检查。为此，minifilter驱动程序可以使用`IS_ALIGNED`宏。**
- OutputBufferLength OutputBuffer指向缓冲区的大小
- ReturnOutputBufferLength 接收OutputBuffer指向的缓冲区中返回的字节数


#### 发送消息

minifilter驱动程序使用`FltSendMessage`例程向等待的用户模式应用程序发送消息

```
NTSTATUS FLTAPI FltSendMessage(
  PFLT_FILTER    Filter,
  PFLT_PORT      *ClientPort,
  PVOID          SenderBuffer,
  ULONG          SenderBufferLength,
  PVOID          ReplyBuffer,
  PULONG         ReplyLength,
  PLARGE_INTEGER Timeout
);
```

> - ClientPort 指向连接端口的指针，此参数详情看`FltCreateCommunicationPort`函数的`ConnectNotifyCallback`项，该函数指针中的有一个ClientPort参数。
> - SenderBuffer 指向缓冲区的指针，该缓冲区包含要发送到用户模式程序的消息。
> - SenderBufferLength SenderBuffer指向缓冲区的大小
> - ReplyBuffer 指向缓冲区的指针，该缓冲区从用户模式程序接收回复。
> - ReplyBufferLength ReplyBuffer指向缓冲区的大小
> - Timeout 指向超时时间的指针，时间单位为100纳秒，在应用程序接收到消息之前，驱动程序会进入等待状态，直到在给定的时间内收到回复，如果设置为NULL的话，则minifilter会进入无限等待状态

当minifilter驱动程序向用户模式应用程序使用`FltSendMessage`发送消息后，如果应用程序调用`FilterGetMessage`来获取消息，则minifilter驱动程序会立即传递消息。在传递消息后，如果`ReplyBuffer`为`NULL`，则`FltSendMessage`返回`STATUS_SUCCESS`。否则，如果`Timeout`非零，则minifilter会进入等待状态：
- 如果应用程序在超时间隔到期之前调用`FilterReplyMessage`，则minifilter驱动程序将收到回复，并且`FltSendMessage`将返回`STATUS_SUCCESS`
- 否则，minifilter驱动程序不会收到回复，`FltSendMessage`将返回`STATUS_TIMEOUT`

其中用户模式的函数详见[用户模式](###用户模式)


### 用户模式

- FilterConnectCommunicationPort
- FilterGetMessage
	
	```
	HRESULT FilterGetMessage(
	  HANDLE                 hPort,
	  PFILTER_MESSAGE_HEADER lpMessageBuffer,
	  DWORD                  dwMessageBufferSize,
	  LPOVERLAPPED           lpOverlapped
	);
	```
	- hPort 调用`FilterConnectCommunicationPort`返回的通信端口句柄。
	- lpMessageBuffer 指向缓冲区的指针，该缓冲区接收来自minifilter的消息。消息必须包含`FILTER_MESSAGE_HEADER`结构。


- FilterReplyMessage
	
	```
	HRESULT FilterReplyMessage(
	  HANDLE               hPort,
	  PFILTER_REPLY_HEADER lpReplyBuffer,
	  DWORD                dwReplyBufferSize
	);
	```
	- lpReplyBuffer 指向缓冲区的指针，该缓冲区包含要发送给minifilter的回复。消息必须包含`FILTER_REPLY_HEADER`结构

- FilterSendMessage

	```
	HRESULT FilterSendMessage(
	  HANDLE  hPort,
	  LPVOID  lpInBuffer,
	  DWORD   dwInBufferSize,
	  LPVOID  lpOutBuffer,
	  DWORD   dwOutBufferSize,
	  LPDWORD lpBytesReturned
	);
	```



## 重要函数

1. FltGetDiskDeviceObject
返回一个与给定的卷相关联的磁盘设备对象
```
NTSTATUS FLTAPI FltGetDiskDeviceObject(
  PFLT_VOLUME    Volume,
  PDEVICE_OBJECT *DiskDeviceObject
);
```
> 此函数增加了`*DiskDeviceObject`中返回的设备对象指针的引用计数。当不再需要此指针时，调用者必须通过调用`ObDereferenceObject`来减少此引用计数。如果不这样做，则会导致系统释放或删除设备对象失败。

2. FltGetDeviceObject
返回一个指向给定卷的过滤管理器的卷设备对象(The FltGetDeviceObject routine returns a pointer to the Filter Manager's volume device object (VDO) for a given volume.)
```
NTSTATUS FLTAPI FltGetDeviceObject(
  PFLT_VOLUME    Volume,
  PDEVICE_OBJECT *DeviceObject
);
```
> 此函数在使用过后，如不再使用DeviceObject，也需要调用`ObDereferenceObject`来减少引用计数。

3. FltFlushBuffers
minifilter给指定的文件刷新缓存

4. FltQueryInformationFile
获取指定文件的信息
