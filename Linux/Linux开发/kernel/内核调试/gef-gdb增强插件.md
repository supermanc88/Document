# GEF - GDB Enhanced Features



gef文档：https://gef.readthedocs.io/en/dev/

中文翻译文档：https://www.lhyerror404.cn/2019/05/29/gef-%E4%BD%BF%E7%94%A8%E6%89%8B%E5%86%8C/



## 安装

要求：gdb版本7.7及以上并且要支持python3

```shell
# via the install script
$ wget -q -O- https://github.com/hugsy/gef/raw/master/scripts/gef.sh | sh

# manually
$ wget -O ~/.gdbinit-gef.py -q https://github.com/hugsy/gef/raw/master/gef.py
$ echo source ~/.gdbinit-gef.py >> ~/.gdbinit
```



项目链接：https://github.com/hugsy/gef

*Note*: As of January 2020, GEF doesn't officially support Python 2 any longer, due to Python 2 becoming officially deprecated. If you really need GDB+Python2, use [`gef-legacy`](https://github.com/hugsy/gef-legacy) instead.



如图为安装成功

![image-20201211175611650](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201211175611650.png)



## 运行

```shell
gdb -q vmlinux 
```



运行这个插件之后，会有一些慢



linux内核调试使用这个插件的话，感觉不是很好



## 编辑布局

```shell
gef> gef config context.layout
---------------------------- GEF configuration setting: context.layout -------------------------------------
context.layout (str) = "legend regs stack code args source memory threads trace extra"
```



目前有6个部分可以显示：

- `legend` : 颜色代码的文字说明
- `regs` : 寄存器的状态
- `stack` : `$sp` 寄存器指向的内存内容
- `code` : 正在执行的代码
- `args` : 如果在函数调用处停止，则打印调用参数
- `source` : 如果用source编译，这将显示相应的源代码行
- `threads` : 所有线程
- `trace` : 执行调用跟踪
- `extra` : 如果检测到漏洞（易受攻击的格式字符串，堆漏洞等），它将显示在此窗格中
- `memory` : 查看任意内存位置

要隐藏一个部分，只需使用`context.layout`设置，并在部分名称前加上`-`或者省略它。

```
gef➤ gef config context.layout "-legend regs stack code args -source -threads -trace extra memory"
```

此配置不会显示`source`，`threads`和`trace`部分。



我自己的配置，不要颜色说明、线程显示和漏洞检测

```shell
gef config context.layout "-legend regs stack code args source memory -threads trace -extra"
```



![image-20201211183754742](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201211183754742.png)



经过一段时间的使用，发现 `regs`、`stack`、`memory`这几个也不怎么常用，所以最终的配置为：

```shell
gef config context.layout "-legend -regs -stack code args source -memory -threads trace -extra"
```



有的时候`trace`这项也会很耗费时间，按需使用。





## 显示效果

结构体的显示也比只用gdb显示好多了

![image-20201217114218500](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201217114218500.png)