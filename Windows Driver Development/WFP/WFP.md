# WFP (Windows Filtering Platform)

## 核心

> 内核态过滤引擎是整个WFP框架的核心。内核态过滤引擎需要和系统网络协议栈交互，通过一种称之为“垫片”的内核模块从网络协议栈中获取网络数据，垫片被插入到网络协议栈各个层中。垫片获取到数据后，通过内核态过滤引擎(KM Filter Engine)提供的分类API（Classify API），把数据传送到WFP的相应分层（Layer）中。

> KM Filter Engine 被划分中若干个分层，在每个分层中，可能会存在一个或多个子层，子层是更小的划分，它们被赋予了不同的权重（weight），在同一个层中，WFP按照子层权重的大小顺序，从大到小把数据交给相应的子层。

WFP中还包含一个重要的数据结构，被称为过滤器（Filter），过滤器中保存了网络数据包的拦截规则和处理动作。

## 垫片

垫片除了能从网络协议栈获取网络数据，还胡更重要的一个作用是把内核态过滤引擎的过滤结果反馈给网络网络协议栈。

垫片负责WFP的数据来源以及执行数据拦截/放行的最终动作，属于网络协议栈和WFP框架之间的通信桥梁。

## Callout接口（呼出接口）

> A callout provides functionality that extends the capabilities of the Windows Filtering Platform. 

以上为官方解释，callout扩展WFP功能。故用`呼出接口`来称呼不合适，callout可解释成`call-out`调用外部。

Callout 由一组 `Callout Function`和一个用来识别一个Callout的`GUID`值

系统内置了几个Callout以供使用，也可以自己通过callout driver添加Callout。

当过滤器关联一个Callout后

- 如果规则被命中时，过滤引擎会回调Callout的`classifyFn`函数。

- 当过滤器被添加到过滤引擎中或者从过滤引擎中移除时，WFP会调用这个callout的`notifyFn1`函数。（主要是看是否承认这个事实）

- 当一个网络数据流将要被终止时，WFP会调用`flowDeleteFn`函数。但是，WFP调用flowDeleteFn是有条件的，只有在这个将要终止的数据流被关联了上下文的情况下，flowDeleteFn都会被调用。

### Callout注册与卸载

注册Callout可以使用`FwpsCalloutRegister0`(Vista开始支持)、`FwpsCalloutRegister1(Win7)`、`FwpsCalloutRegister2(Win8)`

卸载使用：

- `FwpsCalloutUnregisterById0` 使用的是运行注册函数返回的calloutid

- `FwpsCalloutUnregisterByKey0` 使用的是创建callout结构体时填充的calloutkey

### Callout的添加与移除

成功注册callout后，还需要把callout添加到过滤引擎中。

添加之前首先要(`FwpmEngineOpen`)打开过过滤引擎，主要是获取引擎句柄。

使用`FwpmCalloutAdd`添加。

## 分层

分层是一个**容器**，里面包含了零个或多个过滤器。此外，分层内部也可能包含一个或多个子层。

### 子层

使用`FwpmSubLayerAdd`添加子层。

## 过滤器

**过滤器存在于WFP的分层中。**

> 过滤器将子层和callout连接在一起。

过滤器是`规则`和`动作`的集合

- 规则： 指明了对哪些网络数据包感兴趣。它实际上是过滤条件（condition），一个过滤器里面包含一个或多个过滤条件。当所有的条件命中时，才认为规则成立。一旦规则命中，则会执行指定的动作。
- 动作： 拦截/放行

一个分层中可以存在多个过滤器，不同的过滤器被赋予不同的权重。

过滤器定义了几种用于过滤TCP / IP网络数据的过滤条件，如果所有过滤条件都为真，则对数据采取的操作。 如果过滤器需要对网络数据进行额外处理，则可以为过滤器的操作指定Callout。 如果此类过滤器的过滤条件都为真，则过滤器引擎将网络数据传递到指定的Callout以进行其他处理。

### 过滤器添加

使用`FwpmFilterAdd`添加。

## 官方说明

https://docs.microsoft.com/en-us/windows-hardware/drivers/network/windows-filtering-platform-callout-drivers2

