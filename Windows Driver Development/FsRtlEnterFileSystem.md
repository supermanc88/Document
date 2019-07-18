在做文件系统过滤驱动时，会经常用到FsRtlEnterFileSystem函数

   在什么地方调用？

    每一个文件系统驱动例程在进行一个文件读写请求时，需要先调用FsRtlEnterFileSystem，同时在完成请求时，要立即调用FsRtlExitFileSystem。

    而文件系统过滤驱动基本上不用调用FsRtlEnterFileSystem，只有在你调用ExAcquireResourceExclusive(Lite)时，才需要调用，同时在调用完ExReleaseResource后，要立即调用FsRtlExitFileSystem。

 调用的目的？

     把IRQL Level提高到 APC_LEVEL，来禁止一般的内核APC例程。

 注意事项？

    需啊哟在IRQL PASSIVE_LEVEL级别上调用。
--------------------- 
作者：alwaysrun 
来源：CSDN 
原文：https://blog.csdn.net/alwaysrun/article/details/2290711 
版权声明：本文为博主原创文章，转载请附上博文链接！