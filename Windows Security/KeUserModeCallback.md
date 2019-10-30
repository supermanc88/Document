## KeUserModeCallback过程

nt!KeUserModeCallback  ->  nt!KiCallUserMode  ->  nt!KiServiceExit  ->  ntdll!KiUserCallbackDispatcher  ->  int2B  -> nt!KiCallbackReturn  ->  nt!KeUserModeCallback

这是一个从 ring0 -> ring3 -> ring0 的过程。


在准备堆栈完毕后，借用`KiServiceExit`的力量回到了`ring3`，它的着陆点是`KiUserCallbackDispatcher`，然后`KiUserCallbackDispatcher`从`PEB`中取出`KernelCallbackTable`的基址，再以`ApiIndex`作为索引在这个表中查找对应的回调函数并调用，调用完之后再`int2B`触发`nt!KiCallbackReturn`两次进入内核，修正堆栈后跳回`KeUserModeCallback`，完成调用。