# Linux Shell



## Shell 中的冒号

在Linux系统中，冒号(:)常用来做路径的分隔符（PATH），数据字段的分隔符（/etc/passwd）等。其实，冒号(:)在Bash中也是一个内建命令，它啥也不做，是个空命令、只起到占一个位置的作用，但有时候确实需要它。当然，它也有它的用途的，否则没必要存在。在·Linux的帮助页中说它除了参数扩展和重定向之外不产生任何作用。



### 常用格式

- `:`

  啥也不做，只起到占位符的作用。比如在编写脚本的过程中，某些语法结构需要多个部分组成，但开始阶段并没有想好或完成相应的代码，这时就可以用:来做占位符，否则执行时就会报错。

  ```shell
  if [ "today" == "2011-08-29" ]; then  
      :  
  else  
      :  
  fi  
  ```

- `: your comment here`

  \# your comment here

  写代码注释（单行注释）。

- 多行注释

  ```shell
  : 'comment line1
  
  comment line2
  
  more comments'
  ```

- `: >file`

  清空文件file的内容。

- `: ${VAR:=DEFAULT}`

  当变量VAR没有声明或者为NULL时，将VAR设置为默认值DEFAULT。如果不在前面加上:命令，那么就会把${VAR:=DEFAULT}本身当做一个命令来执行，报错是肯定的。



### 示例

示例一 参数扩展 

```shell
[root@node56 ~]# : abc=1234 
[root@node56 ~]# echo $abc 

[root@node56 ~]# : ${abc:=1234} 
[root@node56 ~]# echo $abc  
1234

[root@node56 ~]# ${abc:=1234} 
-bash: 1234: command not found
[root@node56 ~]#
```



 

示例二 清空文件 

```shell
[root@node56 ~]# cat <<<"Hello" >123.txt 
[root@node56 ~]# cat 123.txt 
Hello
[root@node56 ~]# : >123.txt 
[root@node56 ~]# cat 123.txt 
[root@node56 ~]#
```

 

示例三 脚本注释、占位符 

脚本test_colon.sh

```shell
#!/bin/sh  
  
: this is single line comment  
  
: 'this is a multiline comment,  
second line  
end of comments'  
  
if [ "1" == "1" ]; then  
        echo "yes"  
else  
        :  
fi  
```

[root@node56 ~]# ./test_colon.sh 
yes



## sed

sed编辑器被称作`流编辑器(steam editor)`，





## Shell 流程控制

### if else

- if

```shell
if condition
then
    command1 
    command2
    ...
    commandN 
fi
```

- if else

```shell
if condition
then
    command1 
    command2
    ...
    commandN
else
    command
fi
```

- if else-if else

```shell
if condition1
then
    command1
elif condition2 
then 
    command2
else
    commandN
fi
```

#### 1 字符串判断

| 条件表达式   | 含义                           |
| ------------ | ------------------------------ |
| str1 = str2  | 当两个串有相同内容、长度时为真 |
| str1 != str2 | 当串str1和str2不等时为真       |
| -n str1      | 当串的长度大于0时为真(串非空)  |
| -z str1      | 当串的长度为0时为真(空串)      |
| str1         | 当串str1为非空时为真           |

#### 2 数字的判断

| 条件表达式    | 含义                 |
| ------------- | -------------------- |
| int1 -eq int2 | 两数相等为真         |
| int1 -ne int2 | 两数不等为真         |
| int1 -gt int2 | int1大于int2为真     |
| int1 -ge int2 | int1大于等于int2为真 |
| int1 -lt int2 | int1小于int2为真     |
| int1 -le int2 | int1小于等于int2为真 |

#### 3 文件的判断

| 条件表达式 | 含义                                        |
| ---------- | ------------------------------------------- |
| -r file    | 用户可读为真                                |
| -w file    | 用户可写为真                                |
| -x file    | 用户可执行为真                              |
| -f file    | 文件为正规文件为真                          |
| -d file    | 文件为目录为真                              |
| -c file    | 文件为字符特殊文件为真                      |
| -b file    | 文件为块特殊文件为真                        |
| -s file    | 文件大小非0时为真                           |
| -t file    | 当文件描述符(默认为1)指定的设备为终端时为真 |

#### 4 复杂逻辑判断

| 条件表达式 | 含义 |
| ---------- | ---- |
| -a         | 与   |
| -o         | 或   |
| !          | 非   |