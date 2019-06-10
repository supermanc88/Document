利用驱动实现文件的隐藏主要是在IRP_MJ_DIRECTORY_CONTROL的PostOperation回调中处理其参数`FLT_CALLBACK_DATA`结构中的缓冲区数据。

```
Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer（或MdlAddress）
```


