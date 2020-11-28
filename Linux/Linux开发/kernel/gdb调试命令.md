# gdb调试命令



## 断点

### 设置断点

- b function_name

- b *address

  **注意：“*”号是必须加在地址前面的，否则会认为是行号**

- b line_num

### 删除断点

- clear + 要清除的断点行号
- delete + 要清除的断点编号

### 禁用/启用断点

- disable/enable + 断点编号



## 变量

### 打印变量

- p variable

### 设置变量值

- set + 变量 = 变量值			改变程序中某个变量的值

### 变量类型

- whatis + 变量		显示某个表达式的数据类型



## 当前EIP或RIP

- where



## 流程控制

如果有时调试太快，错过了想要调试的位置，当你的代码环境支持时，你可以`jump`到那行，再执行一次。

- j + 行号

执行到当前函数返回

- finish

强制返回当前函数

- return

执行到指定位置

- until + 行号



## 汇编

- x
- display
- disassemble

