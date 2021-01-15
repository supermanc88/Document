# Radare2

官方文档： [Introduction - The Official Radare2 Book](https://book.rada.re/index.html)



说起逆向，你想到的可能是IDA Pro，OllyDBG。

而Radare*2*是一款开放源代码的逆向工程平台，它的强大超越你的想象，包括反汇编、分析数据、打补丁、比较数据、搜索、替换、虚拟化等等，同时具备超强的脚本加载能力，并且可以运行在几乎所有主流的平台（GNU/Linux, .Windows *BSD, iOS, OSX, Solaris…）上。可谓是一大神器。



## rabin2

radare2里面有个很牛逼的工具：rabin2

 rabin2 可以获取包括ELF, PE, Mach-O, Java CLASS文件的区段、头信息、导入导出表、字符串相关、入口点等等，并且支持几种格式的输出文件。使用man rabin2 可以查看rabin2的使用帮助文档。

```shell
man rabin2
```



![image-20201230155609153](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201230155609153.png)



![image-20201230155713077](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201230155713077.png)



显示section

```shell
rabin2 -S bin
```



![image-20201230160101733](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201230160101733.png)



## 常用命令



### seeking 定位

1. 定位

   ```shell
   [0x00400410]> s 0x0
   [0x00000000]>
   ```

   

2. 查看当前位置

   ```shell
   [0x00000000]> s
   0x0
   [0x00000000]>
   ```

3. 向前定位N

   ```shell
   [0x00000000]> s+ 128
   [0x00000080]>
   ```

4. 撤销前两次定位

   ```shell
   [0x00000000]> s-
   [0x00000000]> s-
   [0x00400410]>
   ```



### block size 块大小

block size用来确定radare2在没有显示的指出要操作多少字节时，使用多少字节。

使用`b`命令来修改block size

```shell
[0x00000000]> b 0x100   # block size = 0x100
[0x00000000]> b+16      #  ... = 0x110
[0x00000000]> b-32      #  ... = 0xf0
```



### Mapping Files 文件映射

使用命令`o`打开一个文件。如果是一个可识别的二进制文件，则映射到对应的虚拟地址，其它的会映射到地址0.



有时我们需要重建一个二进制或想把文件重新映射到别的地址。在启动r2时，使用`-B`参数进行基地址修改。但是需要注意的是，当打开一个不认识的文件时，需要使用`-m`参数（或当做`o`命令的参数)

