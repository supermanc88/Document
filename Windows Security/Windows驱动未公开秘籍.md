# Windows驱动未公开秘籍

## Driver_Object

`Driver_Object`结构体中有一个`DriverSection`成员。该成员为指向结构体`LDR_DATA_TABLE_ENTRY`的指针。

以下提供的结构体可能不准确，来自react os
```
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY     LoadOrder;
    LIST_ENTRY     MemoryOrder;
    LIST_ENTRY     InitializationOrder;
    PVOID          ModuleBaseAddress;
    PVOID          EntryPoint;
    ULONG          ModuleSize;
    UNICODE_STRING FullModuleName;
    UNICODE_STRING ModuleName;
    ULONG          Flags;
    USHORT         LoadCount;
    USHORT         TlsIndex;
    union {
        LIST_ENTRY Hash;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    ULONG   TimeStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
```

这个结构体可以用来干什么？


1. `LoadOrder`是一个`LIST_ENTRY`链表，用来连接系统中的内核模块，一般情况下，`thisModule->LoadOrder.Flink`指向的就是`ntoskrnl`模块。
具体忘记了，也有人说`Ntoskrnl is always first entry in the list`。也就是需要一直向前遍历，直到找到`ntoskrnl`

如果想找某个模块的基址，可遍历这个链表，通过`ModuleName`对比找出`ModulBaseAddress`。

2. 也看到有人用这个去做驱动隐藏功能，实现思路是将当前的驱动模块从`LoadOrder`链表中进行断链。