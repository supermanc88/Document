遇到以下问题：

```shell
kd> u win32k!NtUserSendInput
win32k!NtUserSendInput:
92c915cd ??              ???
                           ^ Memory access error in 'u win32k!NtUserSendInput'
```


OSR解答：

> When looking at session space you need to switch to a process from
the appropriate session. If you just want to disassemble win32k code,
any interactive process will do (e.g. explorer.exe):


win32k.sys其实实现了shadow ssdt的功能函数，在非gui线程中，内存是不可读的。

