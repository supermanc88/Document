## 直接方式读写操作

操作系统会用户模式下的缓冲区所指向的物理内存在内核模式下再次映射一遍。这样，用户模式的缓冲区和内核模式的缓冲区指向的是同一区域的物理内存。

操作系统用内存描述符表(MDL数据结构)来描述虚拟内存缓冲区的物理页面布局。

![image](./images/1545033751(1).jpg)

**注意：用户模式的这段缓冲区在虚拟内存上是连续的，但是在物理内存上可能是离散的。**


MDL记录这段虚拟内存，这段虚拟内存的大小存储在mdl->ByteCount里，这段虚拟内存的第一个**页**地址是`mdl->StartVa`，这段虚拟内存的首地址对于第一个页地址的偏移量是`mdl->ByteOffset`。因此，这段虚拟内存的首地址是`mdl->StartVa+mdl->ByteOffset`。

MDL结构是半透明的，不要直接访问MDL的不透明成员。而是使用以下宏：

- `MmGetMdlVirtualAddress`返回MDL描述的I/O缓冲区的虚拟内存地址。
- `MmGetMdlByteCount`返回I/O缓冲区的大小
- `MmGetMdlByteOffset`返回I/O缓冲区开头的物理页面内的偏移量。
- `IoAllocateMdl`分配MDL
- `IoFreeMdl`释放MDL
- 可以分配一块非分页内存，危重通过调用`MmInittializeMdl`将此内存块格式化为MDL

**The MmInitializeMdl macro initializes the header of an MDL.**

`MmInitializeMdl`不会初始化MDL头之后的数据。对于驱动分配的非分页内存块，使用`MmBuildMdlForNonPagedPool`去初始化这个数据，以描述I/O缓冲区所在的物理内存。

- `MmProbeAndLockPages`检测指定的虚拟内存页，使它们长驻内存并锁定在内存
- `MmUnlockPages`解锁MDL描述的物理页
- `MmGetSystemAddressForMdlSafe`返回一个非分页的由MDL描述的系统空间的虚拟地址。
