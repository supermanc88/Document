利用驱动实现文件的隐藏主要是在IRP_MJ_DIRECTORY_CONTROL的PostOperation回调中处理其参数`FLT_CALLBACK_DATA`结构中的缓冲区数据。

```
Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer（或MdlAddress）
```

```
Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass
```
> Vista或Win7返回的FileInformationClass结构不再是FileBothDirectoryInformation。而是FileidBothDirectoryInformation

对需要隐藏的文件夹进行断链操作：
```
//大致代码如下：
SafeBuffer = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;

currentFileInfo = (PFILE_ID_BOTH_DIR_INFORMATION)SafeBuffer;

previousFileInfo = currentFileInfo;

do
{
	nextOffset = currentFileInfo->NextEntryOffset;

	if(wcscmp(currentFileInfo->FileName, HideFileName) == 0)
	{
		if(nextOffset == 0)
		{
			previousFileInfo->NextEntryOffset = 0;
		}
		else
		{
			previousFileInfo->NextEntryOffset = (ULONG)((PCHAR)currentFileInfo - (PCHAR)previousFileInfo) + nextOffset;
		}
	}
	else
	{
		previousFileInfo = currentFileInfo;
	}
	currentFileInfo = currentFileInfo;
}while(nextOffset != 0);

```
