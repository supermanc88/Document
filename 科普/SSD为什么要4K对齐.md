# SSD为什么要4K对齐



在对磁盘进行分区时，有一个很重要的注意事项，就是要将分区对齐，不对齐可能会造成磁盘性能的下降。尤其是固态硬盘SSD，基本上都要求4K对齐。磁盘读写速度慢还找不到原因？可能就是4K对齐的锅。那么分区对齐究竟是怎么回事？为什么要对齐？如何才能对齐？如何检测是否对齐呢？今天，我们就来说说分区4K对齐这些事。你想知道的都在这里了。

1. [物理扇区的概念](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-1)
2. [分区及其格式化](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-2)
3. [为什么要分区对齐](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-3)
4. 如何才能对齐
   1. [如何检测物理扇区大小](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-4-1)
   2. [对齐到多少个扇区才正确](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-4-2)
   3. [为什么大家都说4K对齐](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-4-3)
   4. [划分分区时如何具体操作分区对齐](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-4-4)
5. [如何检测是否对齐](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-5)
6. [如何纠正未对齐的分区](https://www.diskgenius.cn/exp/about-4k-alignment.php#tit-6)



## 物理扇区的概念

分区对齐，是指将分区起始位置对齐到一定的扇区。我们要先了解对齐和扇区的关系。我们知道，硬盘的基本读写单位是“扇区”。对于硬盘的读写操作，每次读写都是以扇区为单位进行的，最少一个扇区，通常是512个字节。由于硬盘数据存储结构的限制，单独读写1个或几个字节是不可能的。通过系统提供的接口读写文件数据时，看起来可以单独读写少量字节，实际上是经过了操作系统的转换才实现的。硬盘实际执行时读写的仍然是整个扇区。

近年来，随着对硬盘容量的要求不断增加，为了提高数据记录密度，硬盘厂商往往采用增大扇区大小的方法，于是出现了扇区大小为4096字节的硬盘。我们将这样的扇区称之为“物理扇区”。但是这样的大扇区会有兼容性问题，有的系统或软件无法适应。为了解决这个问题，硬盘内部将物理扇区在逻辑上划分为多个扇区片段并将其作为普通的扇区（一般为512字节大小）报告给操作系统及应用软件。这样的扇区片段我们称之为“逻辑扇区”。实际读写时由硬盘内的程序（固件）负责在逻辑扇区与物理扇区之间进行转换，上层程序“感觉”不到物理扇区的存在。

逻辑扇区是硬盘可以接受读写指令的最小操作单元，是操作系统及应用程序可以访问的扇区，多数情况下其大小为512字节。我们通常所说的扇区一般就是指的逻辑扇区。物理扇区是硬盘底层硬件意义上的扇区，是实际执行读写操作的最小单元。是只能由硬盘直接访问的扇区，操作系统及应用程序一般无法直接访问物理扇区。一个物理扇区可以包含一个或多个逻辑扇区（比如多数硬盘的物理扇区包含了8个逻辑扇区）。当要读写某个逻辑扇区时，硬盘底层在实际操作时都会读写逻辑扇区所在的整个物理扇区。

这里说的“硬盘”及其“扇区”的概念，同样适用于存储卡、固态硬盘（SSD）。接下来我们统称其为“磁盘”。它们在使用上的基本原理是一致的。其中固态硬盘在实现上更加复杂，它有“页”和“块”的概念，为了便于理解，我们可以简单的将其视同为逻辑扇区和物理扇区。另外固态硬盘在写入数据之前必须先执行擦除操作，不能直接写入到已存有数据的块，必须先擦除再写入。所以固态硬盘（SSD）对分区4K对齐的要求更高。如果没有对齐，额外的动作会增加更多，造成读写性能下降。

## 分区及其格式化

磁盘在使用之前必须要先分区并格式化。简单的理解，分区就是指从磁盘上划分出来的一大片连续的扇区。格式化则是对分区范围内扇区的使用进行规划。比如文件数据的储存如何安排、文件属性储存在哪里、目录结构如何存储等等。分区经过格式化后，就可以存储文件了。格式化程序会将分区里面的所有扇区从头至尾进行分组，划分为固定大小的“簇”，并按顺序进行编号。每个“簇”可固定包含一个或多个扇区，其扇区个数总是2的n次方。格式化以后，分区就会以“簇”为最小单位进行读写。文件的数据、属性等等信息都要保存到“簇”里面。

## 为什么要分区对齐

为磁盘划分分区时，是以逻辑扇区为单位进行划分的，分区可以从任意编号的逻辑扇区开始。如果分区的起始位置没有对齐到某个物理扇区的边缘，格式化后，所有的“簇”也将无法对齐到物理扇区的边缘。如下图所示，每个物理扇区由4个逻辑扇区组成。分区是从3号扇区开始的。格式化后，每个簇占用了4个扇区，这些簇都没有对齐到物理扇区的边缘，也就是说，每个簇都跨越了2个物理扇区。

![为什么要分区对齐](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-01.png)

由于磁盘总是以物理扇区为单位进行读写，在这样的分区情况下，当要读取某个簇时，实际上总是需要多读取一个物理扇区的数据。比如要读取0号簇共4个逻辑扇区的数据，磁盘实际执行时，必须要读取0号和1号两个物理扇区共8个逻辑扇区的数据。同理，对“簇”的写入操作也是这样。显而易见，这样会造成读写性能的严重下降。

下面再看对齐的情况。如下图所示，分区从4号扇区开始，刚好对齐到了物理扇区1的边缘，格式化后，每个簇同样占用了4个扇区，而且这些簇都对齐到了物理扇区的边缘。

![为什么要分区对齐](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-02.png)

在这样对齐的情况下，当要读取某个簇，磁盘实际执行时并不需要额外读取任何扇区，可以充分发挥磁盘的读写性能。显然这正是我们需要的。

由此可见，对于物理扇区大小与逻辑扇区大小不一致的磁盘，分区4K对齐才能充分发挥磁盘的读写性能。而不对齐就会造成磁盘读写性能的下降。



## 如何才能对齐

通过前述图示的两个例子可以看到，只要将分区的起始位置对齐到物理扇区的边缘，格式化程序就会将每个簇也对齐到物理扇区的边缘，这样就实现了分区的对齐。其实对齐很简单。

### 如何检测物理扇区大小

划分分区时，要想实现4K对齐，必须首先知道磁盘物理扇区的大小。那么如何查询呢？

打开DiskGenius软件，点击要检测的磁盘，在软件界面右侧的磁盘参数表中，可以找到“扇区大小”和“物理扇区大小”。其中“扇区大小”指的是逻辑扇区的大小。如图所示，这个磁盘的物理扇区大小为4096字节，通过计算得知，它包含了8个逻辑扇区。

![DiskGenius查看结果](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-03.png)

### 对齐到多少个扇区才正确

知道了“扇区大小”和“物理扇区大小”，用“物理扇区大小”除以“扇区大小”就能得到每个物理扇区所包含的逻辑扇区个数。这个数值就是我们要对齐的扇区个数的最小值。只要将分区起始位置对齐到这个数值的整数倍就可以了。举个例子，比如物理扇区大小是4096字节，逻辑扇区大小是512字节，那么4096除以512，等于8。我们只要将分区起始位置对齐到8的整数倍扇区就能满足分区对齐的要求。比如对齐到8、16、24、32、... 1024、2048等等。只要这个起始扇区号能够被8整除就都可以。并不是这个除数数值越大越好。Windows系统默认对齐的扇区数是2048。这个数值基本上能满足几乎所有磁盘的4K对齐要求了。



### 为什么大家都说4K对齐

习惯而已。因为开始出现物理扇区的概念时，多数磁盘的物理扇区大小都是4096即4K字节，习惯了就俗称4K对齐了。实际划分分区时还是要检测一下物理扇区大小，因为有些磁盘的物理扇区可能包含4个、8个、16个或者更多个逻辑扇区（总是2的n次方）。知道物理扇区大小后，再按照刚才说的计算方法，以物理扇区包含的逻辑扇区个数为基准，对齐到实际的物理扇区大小才是正确的。如果物理扇区大小是8192字节，那就要按照8192字节来对齐，严格来讲，这就不能叫4K对齐了。



### 划分分区时如何具体操作分区对齐

以DiskGenius软件为例，建立新分区时，在“建立新分区”对话框中勾选“对齐到下列扇区数的整数倍”，然后选择需要对齐的扇区数目，点“确定”后建立的分区就是对齐的了。如下图所示：

![DiskGenius复制文件](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-04.png)

软件在“扇区数目”下拉框中列出了很多的选项，从中选择任意一个大于物理扇区大小的扇区数都是可以的，都能满足对齐要求。软件列出那么多的扇区数选项只是增加了选择的自由度，并不是数值越大越好。使用过大的数值可能会造成磁盘空间的浪费。软件默认的设置已经能够满足几乎所有磁盘的 4K对齐要求。

除了“建立新分区”对话框，DiskGenius软件还有一个“[快速分区](https://www.diskgenius.cn/help/fastpart.php)”功能，其中也有相同的对齐设置。如下图所示：

![注册DiskGenius](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-05.png)

## 如何检测是否对齐

作为一款强大的分区管理软件，DiskGenius同样提供了[分区4K对齐检测](https://www.diskgenius.cn/help/4k-alignment.php)的功能。你可以用它检测一下自己硬盘的分区是否对齐了。使用方法很简单，打开软件后，首先在软件左侧选中要检测的磁盘，然后选择“工具”菜单中的“分区4KB扇区对齐检测”，软件立即显示检测结果，如下图所示：

![注册DiskGenius](https://raw.githubusercontent.com/supermanc88/ImageSources/master/4k-align-06.png)

最右侧“对齐”一栏是“Y”的分区就是对齐的分区，否则就是没有对齐。没有对齐的分区会用红色字体显示。

## 如何纠正未对齐的分区

对于分区已有数据但是又没有对齐的情况，DiskGenius软件目前还没有提供直接的解决方案（相信以后的版本会提供）。大家可以通过DiskGenius软件，采用分步的方法实现4K对齐。具体步骤如下：

1、用“[备份分区](https://www.diskgenius.cn/help/part2file.php)”的功能将未对齐的分区备份到镜像文件中。

2、删除未对齐的分区，然后重新建立，建立时选择4K对齐。

3、用“[从镜像文件还原分区](https://www.diskgenius.cn/help/restorepart.php)”的功能通过第一步备份的镜像文件还原分区数据。



对于没有对齐又没有数据的分区就很简单了，删除再重建就好。

怎么样？通过上面的介绍，你是否对分区4K对齐有了一个比较全面的认识呢？