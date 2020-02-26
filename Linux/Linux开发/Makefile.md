## 规则

Makefile是一些规则的集合

规则如下：
```
<目标> : <前提依赖...>
    <命令>
```

`<目标>`是必须的，`<前提依赖...>`和`<命令>`都是可选的，但是至少有其中的一个，不能都省略。


在默认情况下，不指定目标的情况下，会自动执行第一个目标。

如：

```
tutorial:
	@# todo: have this actually run some kind of tutorial wizard?
	@echo "Please read the 'Makefile' file to go through this tutorial"
```

在不指定的情况下，`make`和`make tutorial`功能一样。


## 抑制输出

在Unix环境下，一般保持一种风格，即“成功则沉默”，如果没有错误，就不会输出信息。

在上面的例子中，`@`符号用来抑制输出，如果没有此符号的话，会有如下输出：

```
# todo: have this actually run some kind of tutorial wizard?
echo "Please read the 'Makefile' file to go through this tutorial"
Please read the 'Makefile' file to go through this tutorial
```

## 变量

在makefile文件中，每一行命令都作为一个单独的shell执行，所以，当设置了一个变量后，它不会在下一行生效。如下：

```
var-lost:
        export foo=bar
        echo "foo = [$$foo]"
```
结果如下：

```
root@ubuntu:/home/superman/LearnLinux# make var-lost
export foo=bar
echo "foo = [$foo]"	
foo = []
```

怎样使变量生效呢：

方式一：

```
var-kept:
	export foo=bar; \
	echo "foo=[$$foo]"
```

方式二：

```
var-kept:
	export foo=bar;echo "foo=[$$foo]"
```

方式三：

```
.ONESHELL:
var-kept:
	export foo=bar
	echo "foo=[$$foo]"
```

## 依赖

现在我们开始做一个依赖其他事物的事情

```
reslut.txt : source.txt
	@echo "building result.txt from source.txt"
	cp source.txt result.txt
```

当我们直接运行`make result.txt`时，我们会收到一个错误：

```
root@ubuntu:/home/superman/LearnLinux# make result.txt
make: *** No rule to make target 'source.txt', needed by 'result.txt'.  Stop.
```

这是因为我们告知了怎样创建`result.txt`，但是没有告知怎么获取`source.txt`,并且此文件也没有在当前目录下。所以在上面的命令后添加如下：

```
source.txt:
	@echo "building source.txt"
	echo "this is the source" > source.txt
```

现在我们可以运行`touch source.txt`，或者编辑文件，`make result.txt`可以重新生成result.txt。

## 多个文件

当我们一个工程有100个.c 文件时，并且每个.c文件都需要转换成.o文件，最后链接所有的.o文件到一个二进制文件。

如果按照上面的依赖规则也可以完成，但是这将是一个很大并且很无聊的工作。

我们可以通用的规则来处理匹配模式的文件，并声明要将其转换成另一种模式的对应文件。

在规则集中，我们可以使用特殊的语法来引用输入文件和输出文件，下面是一些特殊变量：

- `$@` 
	代表这个规则要创建的文件，即目标文件。@像字母a，代表参数(arguments)，当输入`make foo`时，`foo`就是这个参数。

- `$<`
	代表输入的文件(即列表中的一个依赖文件)

- `$^` 
	代表所有的输入文件，上面的那个是单一文件，这个是文件的一个集合

- `$?` 
	代表所有比目标程序更新的文件，为什么要这么做呢？我们可以通过这个找出哪些文件做出了改变

- `$*` 
	代表匹配符 % 匹配的部分，比如匹配 f1.txt中的f1， `$*`就表示f1

- `$(@D)`和`$(@F)` 
	`$(@D)` 和 `$(@F)` 分别指向 `$@` 的目录名和文件名。比如，`$@`是 src/input.c，那么`$(@D)` 的值为 src ，`$(@F)` 的值为 input.c。

- `$(<D)`和`$(<F)`
	`$(<D)`和`$(<F)`分别指向 `$<` 的目录名和文件名。

更多的自动变量访问：https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

下面看一个使用变量的例子：

```
result_using_var.txt: source.txt
	@echo "building result_using_var.txt using the $<< and $$@ vars"
	cp $< $@
```

现在回来说，我们想把100个源文件转换成对应的目标文件，又不愿意一个接一个地把它们全部写出来，我们就可以用一些Shell脚本来生成它们，并将它们保存到变量中。

makefile变量的定义使用`:=`而不是`=`

```
srcfiles := $(shell echo src/{00..99}.txt)
```

怎样在src目录下创建一个文本文件呢？

我们定义一个文件名通过使用`%`占位符作词干，这意味着任何名为`src/*.txt`，它可匹配任何内容

```
src/%.txt:
	@[ -d src ] || mkdir src
	echo $* > $@
```

不必为每个文件去运行make，我们可以定义一个依赖所有文件的`PHONY`目标，并且没有任何规则。

```
source : $(srcfiles)
```

## .PHONY
.PHONY: clean
    - means the word "clean" doesn't represent a file name in this Makefile;
    - means the Makefile has nothing to do with a file called "clean" 
      in the same directory.

就是表示 如果在当前目录下有和make 目标相同的文件名或文件夹时，不理会这些情况，仍然执行Makefile文件中的指令。
