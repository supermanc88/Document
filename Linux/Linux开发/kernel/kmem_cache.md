# kmem_cache



```c
struct kmem_cache_order_objects {
	unsigned long x;
};

/*
 * Slab cache management.
 */
struct kmem_cache {
	/* Used for retriving partial slabs etc */
	unsigned long flags;
	int size;		/* The size of an object including meta data */
	int objsize;		/* The size of an object without meta data */
	int offset;		/* Free pointer offset. */
	struct kmem_cache_order_objects oo;

	/*
	 * Avoid an extra cache line for UP, SMP and for the node local to
	 * struct kmem_cache.
	 */
	struct kmem_cache_node local_node;

	/* Allocation and freeing of slabs */
	struct kmem_cache_order_objects max;
	struct kmem_cache_order_objects min;
	gfp_t allocflags;	/* gfp flags to use on each alloc */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(void *);
	int inuse;		/* Offset to metadata */
	int align;		/* Alignment */
	unsigned long min_partial;
	const char *name;	/* Name (only for display!) */
	struct list_head list;	/* List of slab caches */
#ifdef CONFIG_SLUB_DEBUG
	struct kobject kobj;	/* For sysfs */
#endif

#ifdef CONFIG_NUMA
	/*
	 * Defragmentation by allocating from a remote node.
	 */
	int remote_node_defrag_ratio;
	struct kmem_cache_node *node[MAX_NUMNODES];
#endif
#ifdef CONFIG_SMP
	struct kmem_cache_cpu *cpu_slab[NR_CPUS];
#else
	struct kmem_cache_cpu cpu_slab;
#endif
};
```



在Linux系统中，伙伴系统(buddy system)是以页为单位管理和分配内存的。但是现实的需求却以字节为单位，假如我们需要申请20bytes，总不能分配一页吧！那岂不是严重浪费内存。那么该如何分配呢？slab分配器就应运而生了，专为小内存分配而生。slab分配器分配内存以Byte为单位。但是slab分配器并没有脱离伙伴系统，而是基于伙伴系统分配的大内存进一步细分成小内存分配。



现在假如从伙伴系统分配一页内存供slab分配器管理。对于每个slab分配器来说，就是将这段连续内存平均分成若干大小相等的object进行管理。可以我们总得知道每一个object的size吧！管理的内存页数也是需要知道的吧！不然怎么知道如何分配呢！因此需要一个数据结构管理。那就是struct kmem_cache。



- flags：object分配掩码，例如经常使用的`SLAB_HWCACHE_ALIGN`标志位，代表创建的kmem_cache管理的object按照硬件cache对齐，一切都是为了速度。
- size：分配的object size
- objsize：实际的object size，就是创建`kmem_cache`时候传递进来的参数。和size的关系就是，size是各种地址对齐之后的大小。因此，size要大于等于objsize
- offset：slab分配在管理object的时候采用的方法是：既然每个object在没有分配之前不在乎每个object中存储的内容，那么完全可以在每个object中存储下一个object内存首地址，就形成了一个单链表。很巧妙的设计。那么这个地址数据存储在object什么位置呢？offset就是存储下个object地址数据相对于这个object首地址的偏移。
- oo：低16位代表一个slab中所有object的数量(oo&((1 << 16) - 1))，高16位代表一个slab管理的page数量((2^(oo 16))pages)
- max：看了代码好像就是等于oo
- min：当按照大小分配内存的时候出现内存不足就会考虑min大小方式分配。min只需要可以容纳一个object即可
- allocflags：从伙伴系统分配内存掩码
- inuse：objsize按照word对齐之后的大小
- name：sysfs文件系统显示使用
- list：系统有一个slab_caches链表，所有的slab都会挂入此链表
- node：slab节点。在NUMA系统中，每个node都有一个`struct kmem_cache_node`数据结构。



## 接口

```c
struct kmem_cache *
kmem_cache_create (const char *name, size_t size, size_t align,
	unsigned long flags, void (*ctor)(void *))
```



`keme_cache_create`是创建`kmem_cache`数据结构，参数描述如下：

- name：`kmem_cache`的名称
- size：slba管理对象的大小
- align：slab分配器分配内存的对齐字节数(以align字节对齐)
- flags：分配内存掩码
- ctor：分配对象的构造回调函数

`kmem_cache_destroy`的作用和`kmem_cache_create`相反，就是销毁创建的`kmem_cache`。

`kmem_cache_alloc`是从cachep参数指定的kmem_cache管理的内存缓存池中分配一个对象，其中flags是分配掩码，GFP_KERNEL是不是很熟悉的掩码？

`kmem_cache_free`是`kmem_cache_alloc`的反操作



## 接口如何使用

slab分配器提供的接口该如何使用呢？其实很简单，总结分成以下几个步骤：

1. `kmem_cache_create`创建一个`kmem_cache`数据结构。
2. 使用`kmem_cache_alloc`接口分配内存，`kmem_cache_free`接口释放内存。
3. release第一步创建的`kmem_cache`数据结构。

demo:

```c
/*
 *　This is a demo for how to use kmem_cache_create
 */
void slab_demo(void)
{
    struct kmem_cache *kmem_cache_16 = kmem_cache_create("kmem_cache_16", 16,
            8, ARCH_KMALLOC_FLAGS,
            NULL);
 
    /* now you can alloc memory, the buf points to 16 bytes of memory*/
    char *buf = kmeme_cache_alloc(kmem_cache_16, GFP_KERNEL);
 
    /*
     * do something what you what, don't forget to release the memory after use
*/
    kmem_cache_free(kmem_cache_16, buf);
 
    kmem_cache_destroy(kmem_cache_16);
}
```



1. 首先使用`kmem_cache_create`创建名称为`kmem_cache_16`的`kmem_cache`，该`kmem_cache`主要是描述如何管理一堆对象，其实就是slab的布局。**每个对象都是16字节，并且分配的对象地址按照8字节对齐，也就是说从kmem_cache_16中分配的对象大小全是16字节。不管你要申请多少，反正就是16Bytes。**当然，`kmem_cache_create`仅仅是创建了一个描述slab缓存池布局的数据结构，并没有从伙伴系统申请内存，具体的申请内存操作是在`kmeme_cache_alloc`中完成的。
2. `kmeme_cache_alloc`从`kmem_cache_16`分配一个对象。
3. 内存使用结束记得`kmem_cache_free`释放。
4. 如果不需要这个`kmem_cache`的话，就可以调用`kmem_cache_destroy`进行销毁吧。在释放`kmem_cache`之前要保证从该`kmem_cache`中分配的对象全部释放了，否则无法释放`kmem_cache`。 



## 数据结构之间的关系

什么是slab缓存池呢？我的理解就是使用`struct kmem_cache`结构描述的一段内存就称作一个slab缓存池。一个slab缓存池就像是一箱牛奶，一箱牛奶中有很多瓶牛奶，每瓶牛奶就是一个object。分配内存的时候，就相当于从牛奶箱中拿一瓶。总有拿完的一天。当箱子空的时候，你就需要去超市再买一箱回来。超市就相当于partial链表，超市存储着很多箱牛奶。如果超市也卖完了，自然就要从厂家进货，然后出售给你。厂家就相当于伙伴系统。

![4a471520078976](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4a471520078976.png)



### slab管理object的方法

在图片的左上角就是一个slub缓存池中object的分布以及数据结构和kmem_cache之间的关系。首先一个slab缓存池包含的页数是由oo决定的。oo拆分为两部分，低16位代表一个slab缓存池中object的数量，高16位代表包含的页数。使用kmem_cache_create()接口创建kmem_cache的时候需要指出obj的size和对齐align。也就是传入的参数。kmem_cache_create()主要是就是填充kmem_cache结构体成员。既然从伙伴系统得到(2^(oo >> 16)) pages大小内存，按照size大小进行平分。一般来说都不会整除，因此剩下的就是图中灰色所示。由于每一个object的大小至少8字节，当然可以用来存储下一个object的首地址。就像图中所示的，形成单链表。图中所示下个obj地址存放的位置位于每个obj首地址处，在内核中称作指针内置式。同时，下个obj地址存放的位置和obj首地址之间的偏移存储在kmem_cache的offset成员。两外一种方式是指针外置式，即下个obj的首地址存储的位置位于obj尾部，也就是在obj尾部再分配sizeof(void *)字节大小的内存。对于外置式则offset就等于kmem_cache的inuse成员。





http://www.wowotech.net/memory_management/426.html

