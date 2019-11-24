首先要有.MODEL伪指令

## .MODEL伪指令

语法如下：

```
.MODEL memorydel[,modeloptions]
```

例：

```
.MODEL flat stdcall
.MODEL flat c
```

其中的`C`和`STDCALL`是调用约定。

当使用`STDCALL`时，编译出来的函数名称类似为：

```
_name@n
```

如`AddTwo(int a, int b)`会编译出`_AddTwo@8`。


如果是按照`C`的编译方式，则会编译出`_AddTwo`。

## C/C++调用汇编

在调用之前要先进行**函数声明**:

在`.c`文件调用时：

```
extern int AddTwo(int a, int b);
```

在`.cpp`文件调用时：

```
extern "C" int AddTwo(int a, int b);
```

