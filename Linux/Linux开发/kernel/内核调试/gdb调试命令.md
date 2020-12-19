# gdb调试命令



## 断点

### 设置断点

```shell
b function_name
```

```shell
b *address
```

**注意：“*”号是必须加在地址前面的，否则会认为是行号**

```shell
b line_num
```



指定文件下断点

```shell
b filename:line_num

b filename:function
```



### 删除断点

```shell
clear 要清除的断点行号
```

```shell
delete 要清除的断点编号
```



### 禁用/启用断点

```shell
disable/enable 断点编号
```



### 禁用全部断点

```shell
disable
```



### 查看断点

```shell
info b
```





## 变量

### 局部变量(当前stack frame)

```shell
info locals
```



### 函数参数(当前stack frame)

```shell
info args
```



### 打印变量

```shell
p variable
```



### 设置变量值

```shell
set 变量名 = 变量值			// 改变程序中某个变量的值
```



### 变量类型

```shell
whatis 变量名				// 显示某个表达式的数据类型
```



## 寄存器

### 显示寄存器

```shell
info registers
i r
info r
```

![image-20201208165151595](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201208165151595.png)



### 访问寄存器

寄存器名前加`$`，进行访问

```shell
$rax
$rbx
...
$rip
...
```





## 打印信息

### 打印字符串

```shell
p *array@length				// length是想要查看的长度
```

### 打印结构体

如下sb为super_block结构指针，直接使用p打印会打印指针数值，并不会显示其中成员

![image-20201208164116488](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201208164116488.png)

使用`p *sb`时，这样会显示详细的成员

![image-20201208164337983](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201208164337983.png)



### 强制转换类型打印

下面的例子为打印rax寄存器内容，并强制转换成一个结构体指针打印

![image-20201208165828157](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201208165828157.png)

```shell
p (type)arg
```





## 进程

```shell
info proc pid
```







## 流程控制



### 当前EIP或RIP

当浏览代码，找不到正在调试的地方时，使用`where`命令进行查找当前调试位置：

```shell
where
```



### 单步调试

```shell
next					// 单步跟踪程序，当遇到函数调用时，也不进入此函数体
n

step					// 单步调试如果有函数调用，则进入函数；
s
```



### 跳转到指定位置执行

如果有时调试太快，错过了想要调试的位置，当你的代码环境支持时，你可以`jump`到那行，再执行一次。

需要注意的是：

1. `jump`命令只改变pc的值，所以改变程序执行可能会出现不同的结果，比如变量i的值
2. 通过（临时）断点的配合，可以让你的程序跳到指定的位置，并停下来

```shell
b 行号
j 行号
```



执行到当前函数返回

```shell
finish
```



强制返回当前函数

```shell
return
```



### 执行到指定位置

假如我们在25行停住了，现在想要运行到29行停住，就可以使用until命令（可简写为u）：

```shell
until 行号
```



## 汇编

```shell
x

display

disassemble
```



## TUI模式

### 开启TUI模式

```shell
gdb --tui vmlinux
```

### 关闭TUI模式

```
Ctrl + X + A
```

### 更换激活窗口

```
Ctrl + X + O
```

### 窗口滚动

*PgUp*

激活窗口的内容向上滚动一页 *Scroll the active window one page up.* 

*PgDn*

激活窗口的内容向下滚动一页 *Scroll the active window one page down.* 

*Up*

激活窗口的内容向上滚动一行 *Scroll the active window one line up.* 

*Down*

激动窗口的内容向下滚动一行 *Scroll the active window one line down.* 

*Left*

激活窗口的内容向左移动一列 *Scroll the active window one column left.* 

*Right*

激活窗口的内容向右移动一列 *Scroll the active window one column right.* 

*C-L*

更新屏幕 *Refresh the screen.* 





## 内核调试常用结构

### 文件系统

```shell
// 查看正在操作的文件名
p file->f_path.dentry->d_iname

p dentry->d_iname

// 通过page获取inode信息
p *page->mapping->host 

// 查看文件大小
p inode->i_size

p dentry->d_inode->i_size


```



