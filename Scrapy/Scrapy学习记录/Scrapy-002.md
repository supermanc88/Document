# Scrapy-002 pipeline组件

## 怎样激活pipeline组件

必须在`setting`中修改`ITEM_PIPELINES`,具体配置如下：

```
ITEM_PIPELINES = {
    'myproject.pipelines.PricePipeline': 300,
	'myproject.pipelines.JsonWriterPipeline': 800,
}
```
上述给类设定的值决定了它的执行顺序，从低到高执行，习惯上这个值在0-1000范围内。

## pipeline是用来做什么的

每当scrapy抓取一个item之后，item会被发送到item pipeline，这个item pipeline通过顺序执行几个组件处理它。

每个 item pipelince component是一个python类，它们收到一个item并对它执行操作，同时决定该item是否可以继续通过管道或是被丢弃并且不再处理。item pipeline的典型用法是：

- 清洗HTML数据
- 验证抓取的数据
- 检查是否重复
- 将抓取的数据存储在数据库中

## 具体实现

**每个item pipeline component python类必须继承`process_item(self,item,spider)`方法**


