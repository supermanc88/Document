# Vim 语法高亮

## 基本语法命令

打开语法高亮：`:syntax enable`, 实际上它只是执行如下命令：
```
:source $VIMRUNTIME/syntax/syntax.vim
```

## 语法文件

如果该语言是另一个语言的超集，它可以包含那个语言对应的文件。例如，cpp.vim可以包含c.vim文件：
```
:so $VIMRUNTIME/syntax/c.vim
```
.vim文件通常使用自动命令载入。例如：
```
:au Syntax c	    runtime! syntax/c.vim
:au Syntax cpp   runtime! syntax/cpp.vim
```

## 命名管理

高亮组名用于匹配相同类型事物的高亮项目。它们被链接到用于指定颜色的高亮组。
