# struct page

以下为page的结构体：

```c
/*
 * Each physical page in the system has a struct page associated with
 * it to keep track of whatever it is we are using the page for at the
 * moment. Note that we have no way to track which tasks are using
 * a page, though if it is a pagecache page, rmap structures can tell us
 * who is mapping it.
 *
 * If you allocate the page using alloc_pages(), you can use some of the
 * space in struct page for your own purposes.  The five words in the main
 * union are available, except for bit 0 of the first word which must be
 * kept clear.  Many users use this word to store a pointer to an object
 * which is guaranteed to be aligned.  If you use the same storage as
 * page->mapping, you must restore it to NULL before freeing the page.
 *
 * If your page will not be mapped to userspace, you can also use the four
 * bytes in the mapcount union, but you must call page_mapcount_reset()
 * before freeing it.
 *
 * If you want to use the refcount field, it must be used in such a way
 * that other CPUs temporarily incrementing and then decrementing the
 * refcount does not cause problems.  On receiving the page from
 * alloc_pages(), the refcount will be positive.
 *
 * If you allocate pages of order > 0, you can use some of the fields
 * in each subpage, but you may need to restore some of their values
 * afterwards.
 *
 * SLUB uses cmpxchg_double() to atomically update its freelist and
 * counters.  That requires that freelist & counters be adjacent and
 * double-word aligned.  We align all struct pages to double-word
 * boundaries, and ensure that 'freelist' is aligned within the
 * struct.
 */
#ifdef CONFIG_HAVE_ALIGNED_STRUCT_PAGE
#define _struct_page_alignment	__aligned(2 * sizeof(unsigned long))
#else
#define _struct_page_alignment
#endif

struct page {
	unsigned long flags;		/* Atomic flags, some possibly
					 * updated asynchronously */
	/*
	 * Five words (20/40 bytes) are available in this union.
	 * WARNING: bit 0 of the first word is used for PageTail(). That
	 * means the other users of this union MUST NOT use the bit to
	 * avoid collision and false-positive PageTail().
	 */
	union {
		struct {	/* Page cache and anonymous pages */
			/**
			 * @lru: Pageout list, eg. active_list protected by
			 * pgdat->lru_lock.  Sometimes used as a generic list
			 * by the page owner.
			 */
			struct list_head lru;
			/* See page-flags.h for PAGE_MAPPING_FLAGS */
			struct address_space *mapping;
			pgoff_t index;		/* Our offset within mapping. */
			/**
			 * @private: Mapping-private opaque data.
			 * Usually used for buffer_heads if PagePrivate.
			 * Used for swp_entry_t if PageSwapCache.
			 * Indicates order in the buddy system if PageBuddy.
			 */
			unsigned long private;
		};
		struct {	/* page_pool used by netstack */
			/**
			 * @dma_addr: might require a 64-bit value even on
			 * 32-bit architectures.
			 */
			dma_addr_t dma_addr;
		};
		struct {	/* slab, slob and slub */
			union {
				struct list_head slab_list;
				struct {	/* Partial pages */
					struct page *next;
#ifdef CONFIG_64BIT
					int pages;	/* Nr of pages left */
					int pobjects;	/* Approximate count */
#else
					short int pages;
					short int pobjects;
#endif
				};
			};
			struct kmem_cache *slab_cache; /* not slob */
			/* Double-word boundary */
			void *freelist;		/* first free object */
			union {
				void *s_mem;	/* slab: first object */
				unsigned long counters;		/* SLUB */
				struct {			/* SLUB */
					unsigned inuse:16;
					unsigned objects:15;
					unsigned frozen:1;
				};
			};
		};
		struct {	/* Tail pages of compound page */
			unsigned long compound_head;	/* Bit zero is set */

			/* First tail page only */
			unsigned char compound_dtor;
			unsigned char compound_order;
			atomic_t compound_mapcount;
		};
		struct {	/* Second tail page of compound page */
			unsigned long _compound_pad_1;	/* compound_head */
			unsigned long _compound_pad_2;
			struct list_head deferred_list;
		};
		struct {	/* Page table pages */
			unsigned long _pt_pad_1;	/* compound_head */
			pgtable_t pmd_huge_pte; /* protected by page->ptl */
			unsigned long _pt_pad_2;	/* mapping */
			union {
				struct mm_struct *pt_mm; /* x86 pgds only */
				atomic_t pt_frag_refcount; /* powerpc */
			};
#if ALLOC_SPLIT_PTLOCKS
			spinlock_t *ptl;
#else
			spinlock_t ptl;
#endif
		};
		struct {	/* ZONE_DEVICE pages */
			/** @pgmap: Points to the hosting device page map. */
			struct dev_pagemap *pgmap;
			void *zone_device_data;
			/*
			 * ZONE_DEVICE private pages are counted as being
			 * mapped so the next 3 words hold the mapping, index,
			 * and private fields from the source anonymous or
			 * page cache page while the page is migrated to device
			 * private memory.
			 * ZONE_DEVICE MEMORY_DEVICE_FS_DAX pages also
			 * use the mapping, index, and private fields when
			 * pmem backed DAX files are mapped.
			 */
		};

		/** @rcu_head: You can use this to free a page by RCU. */
		struct rcu_head rcu_head;
	};

	union {		/* This union is 4 bytes in size. */
		/*
		 * If the page can be mapped to userspace, encodes the number
		 * of times this page is referenced by a page table.
		 */
		atomic_t _mapcount;

		/*
		 * If the page is neither PageSlab nor mappable to userspace,
		 * the value stored here may help determine what this page
		 * is used for.  See page-flags.h for a list of page types
		 * which are currently stored here.
		 */
		unsigned int page_type;

		unsigned int active;		/* SLAB */
		int units;			/* SLOB */
	};

	/* Usage count. *DO NOT USE DIRECTLY*. See page_ref.h */
	atomic_t _refcount;

#ifdef CONFIG_MEMCG
	struct mem_cgroup *mem_cgroup;
#endif

	/*
	 * On machines where all RAM is mapped into kernel address space,
	 * we can simply calculate the virtual address. On machines with
	 * highmem some memory is mapped into kernel virtual memory
	 * dynamically, so we need a place to store that address.
	 * Note that this field could be 16 bits on x86 ... ;)
	 *
	 * Architectures with slow multiplication can define
	 * WANT_PAGE_VIRTUAL in asm/page.h
	 */
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */

#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
	int _last_cpupid;
#endif
} _struct_page_alignment;
```



page代表系统内存的最小单位。



`struct page`中使用了大量的联合体。



```c
		struct {	/* Page cache and anonymous pages */
			/**
			 * @lru: Pageout list, eg. active_list protected by
			 * pgdat->lru_lock.  Sometimes used as a generic list
			 * by the page owner.
			 */
			struct list_head lru;
			/* See page-flags.h for PAGE_MAPPING_FLAGS */
			struct address_space *mapping;
			pgoff_t index;		/* Our offset within mapping. */
			/**
			 * @private: Mapping-private opaque data.
			 * Usually used for buffer_heads if PagePrivate.
			 * Used for swp_entry_t if PageSwapCache.
			 * Indicates order in the buddy system if PageBuddy.
			 */
			unsigned long private;
		};
```

文件系统主要用的是这个结构体



- index: 在映射的虚拟空间内的偏移，一个文件可能只映射一部分，假设映射了1M的空间，index指的是在1M空间内的偏移，而不是在整个文件内的偏移。

- mapping: 

  ```c
  /**
   * http://opengrok.theworktips.cloud/source/xref/linux-5.3.13/include/linux/fs.h#442
   * struct address_space - Contents of a cacheable, mappable object.
   * 可缓存、可映射对象的内容
   * @host: Owner, either the inode or the block_device.
   * 内容的所有者，即文件节点
   * @i_pages: Cached pages.
   * 缓存的页面
   * @gfp_mask: Memory allocation flags to use for allocating pages.
   * 用于分配页面的内存分配标志。
   * @i_mmap_writable: Number of VM_SHARED mappings.
   * @i_mmap: Tree of private and shared mappings.
   * @i_mmap_rwsem: Protects @i_mmap and @i_mmap_writable.
   * @nrpages: Number of page entries, protected by the i_pages lock.
   * @nrexceptional: Shadow or DAX entries, protected by the i_pages lock.
   * @writeback_index: Writeback starts here.
   * @a_ops: Methods.
   * @flags: Error bits and flags (AS_*).
   * @wb_err: The most recent error which has occurred.
   * @private_lock: For use by the owner of the address_space.
   * @private_list: For use by the owner of the address_space.
   * @private_data: For use by the owner of the address_space.
   */
  struct address_space {
  	struct inode		*host;
  	struct xarray		i_pages;
  	gfp_t			gfp_mask;
  	atomic_t		i_mmap_writable;
  	struct rb_root_cached	i_mmap;
  	struct rw_semaphore	i_mmap_rwsem;
  	unsigned long		nrpages;
  	unsigned long		nrexceptional;
  	pgoff_t			writeback_index;
  	const struct address_space_operations *a_ops;
  	unsigned long		flags;
  	errseq_t		wb_err;
  	spinlock_t		private_lock;
  	struct list_head	private_list;
  	void			*private_data;
  } __attribute__((aligned(sizeof(long)))) __randomize_layout;
  ```

  

mapping指定了页帧所在的地址空间, index是页帧在映射内部的偏移量. 地址空间是一个非常一般的概念. 例如, 可以用在向内存读取文件时. 地址空间用于将文件的内容与装载数据的内存区关联起来. mapping不仅能够保存一个指针, 而且还能包含一些额外的信息, 用于判断页是否属于未关联到地址空间的某个匿名内存区.

1. 如果mapping = 0，说明该page属于交换高速缓存页（swap cache）；当需要使用地址空间时会指定交换分区的地址空间swapper_space。
2. 如果mapping != 0，第0位bit[0] = 0，说明该page属于页缓存或文件映射，mapping指向文件的地址空间address_space。
3. 如果mapping != 0，第0位bit[0] != 0，说明该page为匿名映射，mapping指向struct anon_vma对象。



## 重点成员结构

- index

  在映射的虚拟空间(vma_area)内的偏移；一个文件可能只映射了一部分，假设映射了1M的空间，index指的是在1M空间内的偏移，而不是在整个文件内的偏移。单位是**SHIFT_SIZE**，就是当前系统一个页的大小。为0的话，就是1M空间的0位置；为1的话就是1M空间内的4096位置。

- flag

  用来存放页的状态。这些状态包括页是不是脏的，是不是被锁定在内存中等。flag中的每一位单独表示一种状态，所以它至少可以同时表示出32种不同的状态。

- count

  存放页的引用计数——也就量这一页被引用了多少次。当计数值变为-1时，就说明当前内核并没有引用这一页，于是，在新的分配中就可以使用它了。

- virtual

  页的虚拟地址。通常情况下，它就是页在虚拟内存中的地址。有些内存(即所谓的高端内存)并不永久地映射到内核地址空间上。在这种情况下，这个域的值为NULL，需要的时候，必须动态地映射这些页。