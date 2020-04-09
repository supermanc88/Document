# Linux驱动模块的依赖

内核模块如果引用到Linux内核中的符号，这个则不属于模块间的依赖，因为内核导出的符号本身就是供内核模块所使用。

本帖要讨论的是在两个独立编译的模块A和B之间，B如果要引用A导出的符号，在Makefile中该如何把这一信息加入的问题。

绝大多数情形下，内核配置时能会启用`CONFIG_MODVERSIONS`，这意味着无论是内核还是内核模块，在导出符号时都会为该符号生成CRC校验码，这个校验码保存在`Module.symvers`文件中。

最常见的是，模块会引用到内核导出的符号，此时模块的Makefile没有什么特殊的地方。现在假设A导出一个符号`A_sym`，那么`A_sym`的CRC校验码会存在于A模块所在目录的`Module.symvers`文件中，如果B模块引用到A模块的`A_sym`符号，那么是需要在它的`'__versions' section`中生成`A_sym`符号的校验码的，这个校验码直接取自于A模块的`Module.symvers`文件。如果B模块在编译时从它的 Makefile中无法获得这一信息，首先编译阶段就会产生一个WARNING，其次加载阶段也会因为符号没有CRC校验码而导致加载失败。

此时我们需要在B模块的Makefile文件中加上下面一行，以告诉模块的编译工具链到何处查找`A_sym`符号的CRC校验码：

KBUILD_EXTMOD ：= A模块所在的目录

如此，modpost工具除了到内核所在目录下查找外，还会到KBUILD_EXTMOD指定的目录下查找Module.symvers，以确定本模块所有未定义符号的CRC值。

最后给一个具体的Makefile:

```
obj-m := dep_on_A.o
KERNELDIR := /lib/modules/$(shell uname -r)/build
KBUILD_EXTMOD := /home/dennis/workspace/Linux/book/kmodule/A_mod
PWD := $(shell pwd)
default:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
```

## 内核模块导出符号表 

示例：

首先编写一个导出函数的模块 module_A: test_module_A.c

```
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>

MODULE_LICENSE("Dual BSD/GPL");

static int test_export_A_init(void)
{
    /* 进入内核模块 */
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "ENTRY test_export_A!\n");
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "\n\n\n\n\n");
    return 0;
}

static void test_export_A_exit(void)
{
    /* 退出内核模块 */
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "EXIT test_export_A!\n");
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "\n\n\n\n\n");
}

/* 要导出的函数 */
int export_add10(int param)
{
    printk(KERN_ALERT "param from other module is : %d\n", param);
    return param + 10;
}
EXPORT_SYMBOL(export_add10);

module_init(test_export_A_init);
module_exit(test_export_A_exit);
```

test_module_A.c 的 Makefile

```
# must complile on customize kernel
obj-m += export_A.o
export_A-objs := test_export_A.o

#generate the path
CURRENT_PATH:=$(shell pwd)
#the current kernel version number
LINUX_KERNEL:=$(shell uname -r)
#the absolute path
LINUX_KERNEL_PATH:=/usr/src/kernels/$(LINUX_KERNEL)
#complie object
all:
    make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
    rm -rf modules.order .*.cmd *.o *.mod.c .tmp_versions *.unsigned
#clean
clean:
    rm -rf modules.order Module.symvers .*.cmd *.o *.mod.c *.ko .tmp_versions *.unsigned
```

再编写一个内核模块 module_B，使用 module_A 导出的函数 : test_module_B.c

```
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>

MODULE_LICENSE("Dual BSD/GPL");

extern int export_add10(int);   // 这个函数是 module_A 中实现的

static int test_export_B_init(void)
{
    /* 进入内核模块 */
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "ENTRY test_export_B!\n");
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "\n\n\n\n\n");

    /* 调用 module_A 导出的函数 */
    printk(KERN_ALERT "result from test_export_A: %d\n", export_add10(100));
    
    return 0;
}

static void test_export_B_exit(void)
{
    /* 退出内核模块 */
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "EXIT test_export_B!\n");
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "\n\n\n\n\n");
}

module_init(test_export_B_init);
module_exit(test_export_B_exit);
```

test_module_B.c 的 Makefile

```
# must complile on customize kernel
obj-m += export_B.o
export_B-objs := test_export_B.o

#generate the path
CURRENT_PATH:=$(shell pwd)
#the current kernel version number
LINUX_KERNEL:=$(shell uname -r)
#the absolute path
LINUX_KERNEL_PATH:=/usr/src/kernels/$(LINUX_KERNEL)
#complie object
all:
    make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
    rm -rf modules.order Module.symvers .*.cmd *.o *.mod.c .tmp_versions *.unsigned
#clean
clean:
    rm -rf modules.order Module.symvers .*.cmd *.o *.mod.c *.ko .tmp_versions *.unsigned
```


测试方法:

1. 将 test_export_A.c 和对应的 Makefile 拷贝到 module_A 文件夹中

2. 将 test_export_B.c 和对应的 Makefile 拷贝到 module_B 文件夹中

3. 编译 module_A 中的 test_export_A.c

4. **将编译 module_A 后生成的 Module.symvers 拷贝到 module_B 文件夹中(重点，或者在makefile中指名查找目录)**

5. 编译 module_B 中的 test_export_B.c

6. 先安装 模块A，再安装模块B

7. dmesg 查看log

8. 用 rmmod 卸载模块B 和 模块A (注意卸载顺序，先卸载B再卸载A)