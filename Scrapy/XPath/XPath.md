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


## 谓语

谓语用来查找某个特定的节点或者包含某个指定的值的节点。
**谓语被嵌在方括号中**

例：

|路径表达式|结果|
|---|---|
|/bookstore/book[1]|选取bookstore子元素的第一个book元素|
|/bookstore/book[last()]|选取bookstore子元素的最后一个book元素|
|/bookstore/book[last()-1]|选取bookstore子元素的倒数第二个book元素|
|/bookstore/book[position()<3]|选取最前面的两个属于bookstore元素的子元素的book元素|
|//title[@lang]|选取拥有名为lang属性的title元素|
|//title[@lang='eng']|选取所有的title元素，且lang属性的值为eng|
|/bookstore/book[price>35.00]|选取bookstore子元素的所有book元素，肯其中的price元素的值大于35.00|
|/bookstore/book[price>35.00]/title|选取bookstore元素中的book元素的所有的title元素，且其中的price元素的值须大于35.00|

## 选取未知节点

|通配符|描述|
|---|---|
|*|匹配任何元素节点|
|@*|匹配任何属性节点|
|node()|匹配任何类型的节点|

例：
|路径表达式|结果|
|---|---|
|/bookstore/*|选取bookstore元素的所有子元素|
|//*|选取文档中的所有元素|
|//title[@*]|选取所有带有属性的title元素|

## 选取若干路径

通过在路径表达式中使用"|"运算符，可以选取若干个路径。


# XPath轴

轴可定义相对于当前节点的节点集。

|轴名称|结果|
|---|---|
|ancestor|选取当前节点的所有先辈|
|ancestor-or-self|选取当前节点的所有先辈以及本身|
|attribute|选取当前节点的所有属性|
|child|选取当前节点的所有子元素|
|descendant|选取当前节点的所有后代元素|
|descendant-or-self|选取当前节点的所有后代元素以及本身|
|following|选取当前节点的结束标签之后的所有节点|
|namespace|选取当前节点的所有命名空间节点|
|parent|选取当前节点的父节点|
|preceding|选取当前节点的开始标签之前的所有节点|
|preceding-sibling|选取当前节点之前的所有同级节点|
|self|选取当前节点|

## 位置路径表达式

位置路径可以是绝对的，也可以是相对的

```
/step/step/...
step/step/...
```

**step**包括：

- 轴
- 节点测试
- 零个或者多个谓语

step的语法：

> 轴名称::节点测试[谓语]


