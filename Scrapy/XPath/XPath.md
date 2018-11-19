# XPath

XPath是一门在XML文档中查找信息的语言。XPath用于在XML文档中通过元素和属性进行导航。

# XPath 路径表达式

XPath使用路径表达式来选取XML文档中的节点或者节点集。

在XPath中，有七种类型的节点：

- 元素
- 属性
- 文本
- 命名空间
- 处理指令
- 注释
- 文档节点
	
# XPath 术语

## 节点
```
<?xml version="1.0" encoding="ISO-8859-1"?>

<bookstore>

<book>
  <title lang="en">Harry Potter</title>
    <author>J K. Rowling</author> 
	  <year>2005</year>
  <price>29.99</price>
</book>

</bookstore>
```
`<bookstroe>` 文档节点
`<author>jk</author>` 元素结点
`lang="en"` 属性节点

## 基本值
基本值是无父或无子的节点，上述的 j k rowling

## 项目
项目是基本值或者节点

# 节点关系

## 父
book元素是title等元素的父

## 子
title、author等是book元素的子

## 同胞
title、author等元素是同胞

## 先辈
某节点的父、父的父
title元素的先辈是book元素和bookstore元素

## 后代
某个节点的子、子的子


# XPath语法

## 选取节点

|表达式|描述|
|---|---|
|nodename|选取此节点的所有子节点|
|/|从根节选取|
|//|从匹配选择的当前节点选择文档中的节点，而不考虑它们的位置|
|.|选取当前节点|
|..|选取当前节点的父节点|
|@|选取属性|



|路径表达式|结果|
|---|---|
|bookstore|选取 bookstore 元素的所有子节点。|
|/bookstore|选取根元素 bookstore。注释：假如路径起始于正斜杠( / )，则此路径始终代表到某元素的绝对路径！|
|bookstore/book|选取属于 bookstore 的子元素的所有 book 元素。|
|//book|选取所有 book 子元素，而不管它们在文档中的位置。|
|bookstore//book|选择属于 bookstore 元素的后代的所有 book 元素，而不管它们位于 bookstore 之下的什么位置。|
|//@lang|选取名为 lang 的所有属性。|
