Re: 内核文件ntoskrnl.exe, ntkrnlpa.exe, ntkrnlmp.exe, ntkrpamp.exe到底有什么区别？

	
简单来说，是同一套源代码根据编译选项的不同而编译出四个可执行文件，分别用于：
ntoskrnl - 单处理器，不支持PAE
ntkrnlpa - 单处理器，支持PAE
ntkrnlmp - 多处理器，不支持PAE
ntkrpamp - 多处理器，支持PAE
在Vista之前，安装程序会在安装时根据系统的配置选择两个多处理器或者两个单处理器的版本复制到目标系统中。从Vista开始，会统一使用多处理器版本，因为多处理器版本运行在单处理器上只是效率稍微低一些。



```C
NTSTATUS Status;
PUCHAR BaseAddress = NULL;
NTSTATUS ntStatus;
PMODULES pModules;
ULONG NeededSize;
pModules = (PMODULES)&pModules;
ntStatus = NtQuerySystemInformation(SystemModuleInformation, pModules, 4, &NeededSize);
if(ntStatus == STATUS_INFO_LENGTH_MISMATCH)
{
    pModules = (PMODULES)ExAllocatePool(PagedPool, NeededSize);
    if(!pModules)
        return STATUS_INSUFFICIENT_RESOURCES;
    ntStatus = NtQuerySystemInformation(SystemModuleInformation, pModules, NeededSize, NULL);
    if(!NT_SUCCESS(ntStatus))
    {
        ExFreePool(pModules);
        return ntStatus;
    }
}
if(!NT_SUCCESS(ntStatus))
{
    return ntStatus;
}
BaseAddress = (PUCHAR)pModules->smi.Module[0].MappedBase;
```