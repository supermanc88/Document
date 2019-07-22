The `IoRegisterFsRegistrationChange` routine registers a file system filter driver's notification routine to be called whenever a file system registers or unregisters itself as an active file system.

 大致意思为：`IoRegisterFsRegistrationChange`为文件系统过滤驱动注册一个通知例程，当文件系统注册或注销一个活动的文件系统时会被调用。

```C++
NTSTATUS IoRegisterFsRegistrationChange(
  PDRIVER_OBJECT          DriverObject,
  PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
);
```

```C++
//通知函数原型
DRIVER_FS_NOTIFICATION DriverFsNotification;

void DriverFsNotification(
  _DEVICE_OBJECT *DeviceObject,
  BOOLEAN FsActive
)
{...}
```
IoRegisterFsRegistrationChange registers a file system filter driver to be notified whenever a file system calls IoRegisterFileSystem or IoUnregisterFileSystem.

IoRegisterFsRegistrationChange注册文件系统过滤驱动程序，以便在文件系统调用IoRegisterFileSystem或IoUnregisterFileSystem时收到通知。

To stop receiving such notifications, the filter driver should call IoUnregisterFsRegistrationChange.

当停止接收通知时,调用`IoUnregisterFsRegistrationChange`

> 注意在Microsoft Windows XP及更高版本中，当文件系统过滤驱动程序调用IoRegisterFsRegistrationChange时，会立即为所有当前已注册的文件系统调用其通知例程（即，已调用IoRegisterFileSystem但尚未调用IoUnregisterFileSystem的文件系统）。

> 在IoRegisterFsRegistrationChange返回之前也可以调用通知例程，所以驱动程序必须创建了处理这些通知所需的相关数据结构之后才应调用此例程。

> 此外，在Windows XP及更高版本中，IoRegisterFsRegistrationChange会忽略RAW设备。 有关按名称附加到RAW文件系统的信息，请参阅将过滤器设备对象附加到目标设备对象。

`IoRegisterFsRegistrationChange`会增加过滤驱动程序的驱动对象的引用计数。

```C++

NTSTATUS
IoRegisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    )

/*++

Routine Description:

    This routine registers the specified driver's notification routine to be
    invoked whenever a file system registers or unregisters itself as an active
    file system in the system.

Arguments:

    DriverObject - Pointer to the driver object for the driver.

    DriverNotificationRoutine - Address of routine to invoke when a file system
        registers or unregisters itself.

Return Value:

    STATUS_DEVICE_ALREADY_ATTACHED -
                Indicates that the caller has already registered
                last with the same driver object & driver notification

    STATUS_INSUFFICIENT_RESOURCES
    STATUS_SUCCESS

--*/

{
    PNOTIFICATION_PACKET nPacket;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );

    if (!IsListEmpty( &IopFsNotifyChangeQueueHead )) {

        //
        // Retrieve entry at tail of list
        //

        nPacket = CONTAINING_RECORD( IopFsNotifyChangeQueueHead.Blink, NOTIFICATION_PACKET, ListEntry );

        if ((nPacket->DriverObject == DriverObject) &&
            (nPacket->NotificationRoutine == DriverNotificationRoutine)) {

            ExReleaseResourceLite( &IopDatabaseResource);
            return STATUS_DEVICE_ALREADY_ATTACHED;
        }
    }

    //
    // Begin by attempting to allocate storage for the shutdown packet.  If
    // one cannot be allocated, simply return an appropriate error.
    //

    nPacket = ExAllocatePoolWithTag( PagedPool|POOL_COLD_ALLOCATION,
                                     sizeof( NOTIFICATION_PACKET ),
                                     'sFoI' );
    if (!nPacket) {

        ExReleaseResourceLite( &IopDatabaseResource );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize the notification packet and insert it onto the tail of the
    // list.
    //

    nPacket->DriverObject = DriverObject;
    nPacket->NotificationRoutine = DriverNotificationRoutine;

    InsertTailList( &IopFsNotifyChangeQueueHead, &nPacket->ListEntry );

    IopNotifyAlreadyRegisteredFileSystems(&IopNetworkFileSystemQueueHead, DriverNotificationRoutine, FALSE);
    IopNotifyAlreadyRegisteredFileSystems(&IopCdRomFileSystemQueueHead, DriverNotificationRoutine, TRUE);
    IopNotifyAlreadyRegisteredFileSystems(&IopDiskFileSystemQueueHead, DriverNotificationRoutine, TRUE);
    IopNotifyAlreadyRegisteredFileSystems(&IopTapeFileSystemQueueHead, DriverNotificationRoutine, TRUE);

    //
    // Notify this driver about all already notified filesystems
    // registered as an active file system of some type.
    //


    ExReleaseResourceLite( &IopDatabaseResource );

    //
    // Increment the number of reasons that this driver cannot be unloaded.
    //

    ObReferenceObject( DriverObject );

    return STATUS_SUCCESS;
}


VOID
IopNotifyAlreadyRegisteredFileSystems(
    IN PLIST_ENTRY  ListHead,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine,
    IN BOOLEAN SkipRaw
    )
/*++

Routine Description:

    This routine calls the driver notification routine for filesystems
    that have already been registered at the time of the call.

Arguments:

    ListHead - Pointer to the filesystem registration list head.
    DriverNotificationRoutine - Pointer to the routine that has to be called.

Return Value:

    None.

--*/
{
    PLIST_ENTRY entry;
    PDEVICE_OBJECT fsDeviceObject;

    entry = ListHead->Flink;
    while (entry != ListHead) {

        //
        // Skip raw filesystem notification
        //
        if ((entry->Flink == ListHead) && (SkipRaw)) {
            break;
        }

        fsDeviceObject = CONTAINING_RECORD( entry, DEVICE_OBJECT, Queue.ListEntry );
        entry = entry->Flink;
        DriverNotificationRoutine( fsDeviceObject, TRUE );
    }
}

```
> `CONTAINING_RECORD`根据结构体中的某成员的地址来推算出该结构体整体的地址

由上源码可知，当调用`IoRegisterFsRegistrationChange`时，内部会对`IopNetworkFileSystemQueueHead`、`IopCdRomFileSystemQueueHead`、`IopDiskFileSystemQueueHead`、`IopTapeFileSystemQueueHead`这四个ListEntry遍历，它们记录了文件系统的设备。
