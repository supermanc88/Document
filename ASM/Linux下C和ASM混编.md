# Linux下C和ASM混编



### 前言

在某些情况下，我们可能会将C代码与汇编代码一起混合使用。比如，使用汇编代码直接与硬件进行交互，或者在处理任务时希望占用尽量少的资源同时获得最大的性能，而使用C代码处理一些更高级 的任务。通常情况下，混合使用C与汇编可分为以下三种情形：

- 在C中调用汇编中定义的函数
- 在汇编中调用C语言中的函数
- 直接在C语言中嵌入汇编



在介绍C与汇编混合使用之前，先介绍一下在Linux系统中进行系统调用时传参的约定，以及在进行函数调用时的传参约定。



### Linux 系统调用约定 (这块可能不对)

**系统调用**是用户程序与Linux 内核之间的接口，用于让内核执行一些系统任务，如文件访问、进程管理及网络任务等。在Linux中，有多种方式可以用于进行系统调用，这里只介绍通过使用`int $0x80`或`syscall`产生软中断来进行系统调用的方式。该方法比较简单直观，方便在汇编代码中进行系统调用。

#### int $0x80

在Linux x86 和Linux x86_64中，可以直接使用`int $0x80`命令来进行系统调用。以Linux x86为例，参数传递规则如下，其中返回值通过寄存器eax返回。

| 系统调用号 | 参数1 | 参数2 | 参数3 | 参数4 | 参数5 | 参数6 | 返回值 |
| ---------- | ----- | ----- | ----- | ----- | ----- | ----- | ------ |
| eax        | ebx   | ecx   | edx   | esi   | edi   | ebp   | eax    |

系统调用号可以在`/usr/include/asm/unistd_32.h`文件中查看。在系统调用过程中，所有寄存器的值都会保持不变(除了`eax`用于返回值)。

> 由于在Linux x86_64上，寄存器的名称发生了变化，其参数传递规则见下面

#### syscall

在Linux x86_64中引入了一条新的指令`syscall`，与`int $0x80`相比，由于不需要访问中断描述符表，所以会更快。其参数传递规则如下，其中返回值通过寄存器rax返回。

| 系统调用号 | 参数1 | 参数2 | 参数3 | 参数4 | 参数5 | 参数6 | 返回值 |
| ---------- | ----- | ----- | ----- | ----- | ----- | ----- | ------ |
| rax        | rdi   | rsi   | rdx   | r10   | r8    | r9    | rax    |

系统调用号可以在`/usr/include/asm/unistd_64.h`文件中查看。在系统调用过程中，会改变寄存器`rcx`和`r11`的内容，其他寄存器的内容会保持不变(除了`rax`用于返回值)。



### 函数调用传参约定

在Linux x86中，使用gcc编译器进行程序编译时，函数调用时的参数传递规则如下：

- 函数参数通过栈传递，按照从右往左的顺序入栈；
- 函数返回值保存在寄存器`eax`中。

在Linux x86_64中，函数调用时的参数传递规则如下：

- 前6个参数按从左往右的顺序分别通过寄存器`rdi`、`rsi`、`rdx`、`rcx`、`r8`、`r9`，剩下的参数按从右往左的顺序通过栈传递；
- 函数返回值保存在寄存器`rax`中。

> 函数调用时的参数传递规则实际上与**函数调用约定**有关，与编译器无关，常见的函数调用约定包括`c调用约定`、`std调用约定`、`x86 fastcall约定`以及`C++调用约定`等。gcc编译器采用的c调用约定。



### 在C中调用汇编中定义的函数

以Linux x86为例，用汇编语言编写一个hello_world函数，输出”Hello, World!\n”为例，其不需要任何参数，同时也没有返回值，相应的汇编代码如下：

```assembly
.globl hello_world
.type hello_world, @function
.section .data
message: .ascii "Hello, World!\n"
length: .int . - message
.section .text
hello_world:
  mov $4, %eax
  mov $1, %ebx
  mov $message, %ecx
  mov length, %edx
  int $0x80
  ret
```

> 由于使用gcc进行编译，因此汇编代码中使用AT&T语法。如果在用gcc编译时加上`-masm=intel`选项，则可以使用intel语法。当然，也可以使用nasm对汇编语言进行汇编，然后使用gcc完成链接过程，可参考[这里](https://www.devdungeon.com/content/how-mix-c-and-assembly)。

然后编写一个C程序调用该函数，如下：

```c
extern void hello_world();
 
void main()
{
  hello_world();
}
```

使用gcc进行编译，命令如下：

```shell
gcc -m32 hello_world.c hello_world.s -o hello_world
```



下面通过参数传递将”Hello World!”传入到汇编代码中，修改如下：

```assembly
.globl hello_world
.type hello_world, @function
.section .text
hello_world:
  mov $4, %eax
  mov $1, %ebx
  mov 4(%esp), %ecx
  mov $0xd, %edx
  int $0x80
  ret
```

对应的C程序如下：

```c
extern void hello_world(char* value);
 
void main()
{
  hello_world("Hello World!\n");
}
```



### 在汇编中调用C中的函数

以`printf`为例，通过在汇编代码中调用`printf()`函数，示例代码如下：

```assembly
.extern printf
.globl main
.section .data
message: .ascii "hello,world!\n"
format: .ascii "%s"
.section .text
main:
    push $message
    push $format
    mov $0, %eax
    call printf
    add $0x8, %esp
    ret
```

使用gcc编译如下：

```shell
gcc hello_world.s -o hello_world
```

> 1. 使用gcc编译汇编代码时，开始符号不再是_start而是main。由于main是一个函数，所以在最后必须要有`ret`指令；
> 2. 在调用函数之前，寄存器`eax`/`rax`的值必须设为0。



### 在C中嵌入汇编

最直接的方式是在C程序中嵌入汇编代码，以Linux x86_64为例，示例代码如下：

```c
#include <stdio.h>
int sum(int a, int b)
{
  asm("addl %edi, %esi");
  asm("movl %esi, %eax");
}
 
int main()
{
  printf("%d\n", sum(2, 3));
  return 0;
}
```

在上面的示例代码中，也可以将多条汇编指令写在一起，如下：

```c
asm(
    "addl %edi, %esi\n\r"
    "movl %esi, %eax\n\r"
    );
```

**由于gcc编译器在进行解析时是先将汇编指令打印到一个文件中，所以需要带上格式化控制串。**



### 扩展内联汇编 (Extended Asm)

前面讨论的基本内联汇编只涉及到嵌入汇编指令，而在扩展形式中，我们还可以指定操作数，并且可以选择输入输出寄存器，以及指明要修改的寄存器列表。对于要访问的寄存器，并不一定要要显式指明，也可以留给GCC自己去选择，这可能让GCC更好去优化代码。扩展内联汇编格式如下:

```c
asm ( assembler template
        : output operands                /* optional */
        : input operands                   /* optional */
        : list of clobbered registers   /* optional */
);
```





### 小结

对Linux平台下的系统调用及函数调用时的传参约定进行了介绍，同时简单介绍了C与汇编语言混合使用的三种情形。

> 如果想要进行更深入的理解，可自行查阅网上的相关资料。



### 相关链接

- [Mixing Assembly and C](https://abnerrjo.github.io/blog/2016/02/27/mixing-assembly-and-c/)
- [How to Mix C and Assembly](https://www.devdungeon.com/content/how-mix-c-and-assembly)
- [Linux System Calls](https://cs.lmu.edu/~ray/notes/syscalls/)
- [X86 Assembly/Interfacing with Linux](https://en.wikibooks.org/wiki/X86_Assembly/Interfacing_with_Linux)