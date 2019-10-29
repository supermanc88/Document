# makefile

## makefile的规则

```
target ... : prerequisites ...
    command
    ...
    ...
```

`target`： 可以是一个object file，也可以是一个可执行文件，还可以是一个标签(label)。
`prerequisites`： 生成该`target`所依赖的文件
`command`： 该`target`要执行的命令(任意的shell命令)

> `prerequisites`中如果有一个以上的文件比`target`文件要新的话，`command`所定义的命令就会被执行。

以上就是makefile最核心的内容。


## 一个示例

```
edit : main.o kbd.o command.o display.o \
        insert.o search.o files.o utils.o
    cc -o edit main.o kbd.o command.o display.o \
        insert.o search.o files.o utils.o

main.o : main.c defs.h
    cc -c main.c
kbd.o : kbd.c defs.h command.h
    cc -c kbd.c
command.o : command.c defs.h command.h
    cc -c command.c
display.o : display.c defs.h buffer.h
    cc -c display.c
insert.o : insert.c defs.h buffer.h
    cc -c insert.c
search.o : search.c defs.h buffer.h
    cc -c search.c
files.o : files.c defs.h buffer.h command.h
    cc -c files.c
utils.o : utils.c defs.h
    cc -c utils.c
clean :
    rm edit main.o kbd.o command.o display.o \
        insert.o search.o files.o utils.o
```

反斜杠（`\`）是换行符的意思。这样比较便于`makefile`的阅读。我们可以把这个内容保存在名字 为“makefile”或“Makefile”的文件中，然后在该目录下直接输入命令 `make` 就可以生成执行文件edit。如果要删除执行文件和所有的中间目标文件，那么，只要简单地执行一下 `make clean` 就 可以了。

在这个makefile中，目标文件（target）包含：执行文件edit和中间目标文件（ *.o ），依赖文 件（prerequisites）就是冒号后面的那些 .c 文件和 .h 文件。每一个 .o 文件都有 一组依赖文件，而这些 .o 文件又是执行文件 edit 的依赖文件。依赖关系的实质就是说明了目 标文件是由哪些文件生成的，换言之，目标文件是哪些文件更新的。

在定义好依赖关系后，后续的那一行定义了如何生成目标文件的操作系统命令，一定要以一个 `Tab` 键作为开头。


## make是如何工作的

在默认的方式下，也就是我们只输入`make`命令。那么：

1. make会在当前目录下找名字叫`makefile`的文件
2. 如果找到，它会找文件中的第一个目标文件(target)，在上面的例子中为`edit`，并把这个文件作为最终的目标文件
3. 如果`edit`不存在，则找`edit`所依赖的`.o`文件
4. 如果`.o`文件也不存在，则找`.o`所依赖的C文件和H文件
5. 最终make生成`.o`，然后再用`.o`生成`edit`


## clean

像`clean`这种，没有被第一个目标文件直接或间接关联，那么它后面所定义的命令将不会被自动执行，不过我们可以显示地要make执行，即命令`make clean`，以此来清除所有的目标文件，以便重新编译。


## makefile中使用变量

在上面的例子中，先让我们看看edit的规则：

```
edit : main.o kbd.o command.o display.o \
        insert.o search.o files.o utils.o
    cc -o edit main.o kbd.o command.o display.o \
        insert.o search.o files.o utils.o
```

可以看到`.o`文件的字符串被重复了两次，如果我们的工程中需要再加入一个`.o`文件，那么我们需要在两个地方加(应该是三个地方，还有一个在clean中)。当前的makefile文件并不复杂，所以我们可以在这几个地方都加上。但是如果makefile变得复杂，我们就可能会忘掉一个需要加入的地方，而导致编译失败。所以，为了makefile的易维护，在makefile中我们可以使用变量。

比如，我们声明一个变量，叫`objects`，`OBJECTS`, `objs`， ` OBJS`，不管什么，只要能够表示obj文件就行了。我们在makefile一开始就这样定义：

```
objects = main.o kbd.o command.o display.o \
     insert.o search.o files.o utils.o
```

于是，我们就可以很方便地在我们的makefile中以`$(objects)`的方式来使用这个变量，于是改良的makefile就变成下面的这个样子：

```
objects = main.o kbd.o command.o display.o \
    insert.o search.o files.o utils.o

edit : $(objects)
    cc -o edit $(objects)
main.o : main.c defs.h
    cc -c main.c
kbd.o : kbd.c defs.h command.h
    cc -c kbd.c
command.o : command.c defs.h command.h
    cc -c command.c
display.o : display.c defs.h buffer.h
    cc -c display.c
insert.o : insert.c defs.h buffer.h
    cc -c insert.c
search.o : search.c defs.h buffer.h
    cc -c search.c
files.o : files.c defs.h buffer.h command.h
    cc -c files.c
utils.o : utils.c defs.h
    cc -c utils.c
clean :
    rm edit $(objects)
```


## 让makefile自动推导

GNU的make很强大，它**可以自动推导文件以及文件依赖关系后面的命令**，于是我们就没必要去在每一个`.o`文件后都写上类似的命令，因为，我们的make会自动识别，并自己推导命令。

**只要make看到一个`.o`文件，它就会自动的把`.c`文件加在依赖关系中，如果make找到一个`whatever.o`，那么`whatever.c`就会是`whatever.o`的依赖文件。并且`cc -c whatever.c`也会被推导出来。**于是，新的makefile如下：

```
objects = main.o kbd.o command.o display.o \
    insert.o search.o files.o utils.o

edit : $(objects)
    cc -o edit $(objects)

main.o : defs.h
kbd.o : defs.h command.h
command.o : defs.h command.h
display.o : defs.h buffer.h
insert.o : defs.h buffer.h
search.o : defs.h buffer.h
files.o : defs.h buffer.h command.h
utils.o : defs.h

.PHONY : clean
clean :
    rm edit $(objects)
```

这种方法，也就是make的`隐晦规则`。在上面文件内容中，`.PHONY`表示`clean`是个伪目标文件。

## 另类风格的makefile

上面的makefile中看到有多个头文件被多次重复，也可以将它们全部收拢起来：

```
objects = main.o kbd.o command.o display.o \
    insert.o search.o files.o utils.o

edit : $(objects)
    cc -o edit $(objects)

$(objects) : defs.h
kbd.o command.o files.o : command.h
display.o insert.o search.o files.o : buffer.h

.PHONY : clean
clean :
    rm edit $(objects)
```

缺点：虽然这样的makefile变得很简单，但我们的文件依赖关系就显得有点凌乱了，如果文件一多，要加入几个新的`.o`文件，那就理不清了。


## 清空目标文件的规则

每个makefile中都应该写一个清空目标文件的规则（虽然不是必要的），这不仅便于重编译，也很利于保持文件的清洁。

一般的风格：
```
clean:
    rm edit $(objects)
```

更为稳健的做法：

```
.PHONY : clean
clean :
    -rm edit $(objects)
```

前面说过，`.PHONY`表示`clean`是一个`伪目标`。而在`rm`命令前加一个`-`的意思就是，也许某些文件出现问题，但不要管，继续做后面的事。当然，`clean`的规则不要放在文件的开头，不然，这就变成make的默认目标。不成文的规矩是--“clean从来都放在文件的最后”。