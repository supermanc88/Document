# Linux重定向

| 命令            | 说明                                               |
| :-------------- | :------------------------------------------------- |
| command > file  | 将输出重定向到 file。                              |
| command < file  | 将输入重定向到 file。                              |
| command >> file | 将输出以**追加**的方式重定向到 file。              |
| n > file        | 将文件描述符为 n 的文件重定向到 file。             |
| n >> file       | 将文件描述符为 n 的文件以追加的方式重定向到 file。 |
| n >& m          | 将输出文件 m 和 n 合并。                           |
| n <& m          | 将输入文件 m 和 n 合并。                           |
| << tag          | 将开始标记 tag 和结束标记 tag 之间的内容作为输入。 |

*需要注意的是文件描述符 **0** 通常是**标准输入（STDIN）**，**1** 是**标准输出（STDOUT）**，**2** 是**标准错误输出（STDERR）**。*



```shell
>
   # 将stdout重定向到一个文件. 
   # 如果这个文件不存在, 那就创建, 否则就覆盖. 

   ls -lR > dir-tree.list
   # 创建一个包含目录树列表的文件. 

: > filename
   # >操作, 将会把文件"filename"变为一个空文件(就是size为0). 
   # 如果文件不存在, 那么就创建一个0长度的文件(与'touch'的效果相同). 
   # :是一个占位符, 不产生任何输出. 

> filename    
   # >操作, 将会把文件"filename"变为一个空文件(就是size为0). 
   # 如果文件不存在, 那么就创建一个0长度的文件(与'touch'的效果相同). 
   # (与上边的": >"效果相同, 但是某些shell可能不支持这种形式.)

>>
   # 将stdout重定向到一个文件. 
   # 如果文件不存在, 那么就创建它, 如果存在, 那么就追加到文件后边. 

   # 单行重定向命令(只会影响它们所在的行): 


----------


1>filename
   # 重定向stdout到文件"filename". 
1>>filename
   # 重定向并追加stdout到文件"filename". 
2>filename
   # 重定向stderr到文件"filename". 
2>>filename
   # 重定向并追加stderr到文件"filename". 
&>filename
   # 将stdout和stderr都重定向到文件"filename". 

M>N
  # "M"是一个文件描述符, 如果没有明确指定的话默认为1. 
  # "N"是一个文件名. 
  # 文件描述符"M"被重定向到文件"N". 
M>&N
  # "M"是一个文件描述符, 如果没有明确指定的话默认为1. 
  # "N"是另一个文件描述符. 


----------


   # 重定向stdout, 一次一行. 
   LOGFILE=script.log

   echo "This statement is sent to the log file, \"$LOGFILE\"." 1>$LOGFILE
   echo "This statement is appended to \"$LOGFILE\"." 1>>$LOGFILE
   echo "This statement is also appended to \"$LOGFILE\"." 1>>$LOGFILE
   echo "This statement is echoed to stdout, and will not appear in \"$LOGFILE\"."
   # 每行过后, 这些重定向命令会自动"reset". 



   # 重定向stderr, 一次一行. 
   ERRORFILE=script.errors

   bad_command1 2>$ERRORFILE       #  Error message sent to $ERRORFILE.
   bad_command2 2>>$ERRORFILE      #  Error message appended to $ERRORFILE.
   bad_command3                    #  Error message echoed to stderr,
                                   #+ and does not appear in $ERRORFILE.
   # 每行过后, 这些重定向命令也会自动"reset". 

----------


2>&1
   # 重定向stderr到stdout. 
   # 将错误消息的输出, 发送到与标准输出所指向的地方. 

i>&j
   # 重定向文件描述符i到j. 
   # 指向i文件的所有输出都发送到j. 

>&j
   # 默认的, 重定向文件描述符1(stdout)到j. 
   # 所有传递到stdout的输出都送到j中去. 

0< FILENAME
 < FILENAME
   # 从文件中接受输入. 
   # 与">"是成对命令, 并且通常都是结合使用. 
   #
   # grep search-word <filename


[j]<>filename
   # 为了读写"filename", 把文件"filename"打开, 并且将文件描述符"j"分配给它. 
   # 如果文件"filename"不存在, 那么就创建它. 
   # 如果文件描述符"j"没指定, 那默认是fd 0, stdin. 
   #
   # 这种应用通常是为了写到一个文件中指定的地方. 
   echo 1234567890 > File    # 写字符串到"File". 
   exec 3<> File             # 打开"File"并且将fd 3分配给它. 
   read -n 4 <&3             # 只读取4个字符. 
   echo -n . >&3             # 写一个小数点. 
   exec 3>&-                 # 关闭fd 3.
   cat File                  # ==> 1234.67890

|
   # 管道. 
   # 通用目的处理和命令链工具. 
   # 与">", 很相似, 但是实际上更通用. 
   # 对于想将命令, 脚本, 文件和程序串连起来的时候很有用. 
   cat *.txt | sort | uniq > result-file
   # 对所有.txt文件的输出进行排序, 并且删除重复行. 
   # 最后将结果保存到"result-file"中. 



----------


command > filename 　　　　　把标准输出重定向到一个新文件中

command >> filename 　　　　　把标准输出重定向到一个文件中(追加)

command 1 > fielname 　　　　　把标准输出重定向到一个文件中

command > filename 2>&1 　　　把标准输出和标准错误一起重定向到一个文件中

command 2 > filename 　　　　把标准错误重定向到一个文件中

command 2 >> filename 　　　　把标准输出重定向到一个文件中(追加)

command >> filename 2>&1 　　把标准输出和标准错误一起重定向到一个文件中(追加)

command < filename >filename2 　　把command命令以filename文件作为标准输入，以filename2文件作为标准输出

command < filename 　　　把command命令以filename文件作为标准输入

command << delimiter 　　把从标准输入中读入，直至遇到delimiter分界符

command <&m 　　　把文件描述符m作为标准输入

command >&m 　　　把标准输出重定向到文件描述符m中

command <&- 　　　把关闭标准输入 
————————————————
```

