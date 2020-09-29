## KIDTENTRY 结构

```
typedef struct _KIDTENTRY
{
    USHORT Offset;
    USHORT Selector;
    USHORT Access;
    USHORT ExtendedOffset;
}KIDTENTRY,*PKIDTENTRY;
```

INTEL开发手册中描述：

![idt](./images/1573628447(1).jpg)

具体可见开发手册 Vol 3A 6-11