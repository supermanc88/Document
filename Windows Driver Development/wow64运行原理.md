## 32位程序是怎样在windows 64 上运行的？

由于win64内核是64位的，所以32位程序运行时环境在与64位内核时需要有状态转换。

接下来进行分析，可以在`C:/windows/syswow64`中发现许多和`C:/windows/system32`下同名的动态库，如`kernel32.dll`,`ntdll.dll`,`msvcrt.dll`,`ws2_32.dll`等，其实这些都是32位的版本。像wow64名字所表达的含义一样，syswow64这些库相当于在64位windows中构建了一个32位windows子系统环境，我们32位的程序能正常在win64上运行正是靠这个子环境负责与64位环境进行了交互和兼容。

// https://bbs.pediy.com/thread-221236.htm
