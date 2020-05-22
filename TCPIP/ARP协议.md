# ARP协议



## ARP原理之请求应答

ARP(Address Resolution Protocol)地址解析协议，用于实现从IP地址到MAC地址的映射，即询问目标IP对应的MAC地址。

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-482e369fa6a8c4246af0164fbc69e2fc_720w.jpg)



PC1封装数据并且对外发送数据时，上图中出现了"failed"，即数据封装失败了，为什么？

我们给PC1指令-"ping ip2"，这就告知了目的IP，此时PC1便有了通信需要的源目IP地址，但是PC1仍然没有通信需要的目的MAC地址。**这就好比我们要寄一个快递，如果在快递单上仅仅写了收件人的姓名（IP），却没有写收件人的地址（MAC），那么这个快递就没法寄出，因为信息不完整。**



那么，现在PC1已经有了PC2的IP地址信息，如何获取到PC2的MAC地址呢？此时，ARP协议就派上用场了。我们接着上面这张图，继续==>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-12c0720dbf1143fb4ace0cb44398508f_720w.jpg)

通过第三和第四步骤，我们看到PC1和PC2进行了一次ARP请求和回复过程，通过这个交互工程，PC1便具备了PC2的MAC地址信息。

接下来PC1会怎么做呢？在真正进行通信之前，PC1还会将PC2的MAC信息放入本地的【ARP缓存表】，表里面放置了IP和MAC地址的映射信息，例如 IP2<->MAC2。接下来，PC1再次进行数据封装，正式进入PING通信，如下==>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-27582d8469224d8a28ee148cf3543e5e_720w.jpg)

**小结：**经过上面6个步骤的处理，PC1终于把数据包发送出去了，之后便可以进行正常的通信了。看到了吧，ARP的功能和实现过程是如此的简单：它在发送方需要目标MAC地址的时及时出手，通过"一问一答"的方式获取到特定IP对应的MAC地址，然后存储到本地【**ARP缓存表**】，后续需要的话，就到这里查找。



既然是"缓存"表，意味着它有**时效性**，并且如果电脑或者通信设备重启的话，这张表就会**清空**；也就是说，如果下次需要通信，又需要进行ARP请求。在我们的windows/macos系统下，可以通过命令行"**arp -a**"查看具体信息=>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-504474cb1210f7d4e72d718cd86148c8_720w.jpg)



## ARP原理之广播请求单播回应

上面的图解过程看上去简单又纯粹，好像我们就已经把这个协议的精髓全部get到，但其实，我们只是刚揭开了它的面纱，接下来我们才真正进入正题。例如，上面的图解过程中，整个局域网（LAN）只有PC1和PC2两个主机，所以这个一问一答过程非常的顺畅。

而实际网络中，这个LAN可能有几十上百的主机，那么请问，PC1如何将这个【ARP请求包】顺利的交给PC2，而PC2又如何顺利的把【ARP回应包】返回给PC1? 我们看下面的图

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-7f8bdda04b830562b73cf2b3d932f071_720w.jpg)

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-83365e93f2f60f5fb83772eca5dc8016_720w.jpg)

为了营造出"几十上百"的效果，这里多添加了2个主机进来 ⁄(⁄ ⁄•⁄ω⁄•⁄ ⁄)⁄，并且增加了有线和无线的环境。那么，在多主机环境下，PC1现在发出的ARP请求包，怎么交到PC2手里？

这时，ARP协议就需要采用以太网的"广播"功能：将请求包**以广播的形式**发送，交换机或WiFi设备（无线路由器）收到广播包时，会将此数据发给同一局域网的其他所有主机。

那么，什么是广播？对于初学者而言，我们只需要知道，大部分的广播包，它们有一个共同特征：**二层封装时目的MAC是全f（ffff.ffff.ffff）或三层封装时目的IP是全1（255.255.255.255）**。可以这样更方便的记住：目的地址最大的，就是广播。

接下来我们来看下这个ARP广播请求包接下来是如何工作的？

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-336746c18241ff5e5d8eba2b1ffac1da_720w.jpg)

根据上图我们看到，PC1发送的请求广播包同时被其他主机收到，然后PC3和PC4收到之后（发现不是问自己）则丢弃。**而PC2收到之后，根据请求包里面的信息（有自己的IP地址），判断是给自己的，所以不会做丢弃动作，而是返回ARP回应包。**

ARP请求是通过广播方式来实现的，那么，PC2返回ARP回应包，是否也需要通过广播来实现呢？答案是否定的。**大部分网络协议在设计的时候，都需要保持极度克制，不需要的交互就砍掉，能合并的信息就合并，能不用广播就用单播，以此让带宽变得更多让网络变得更快。**

那么，ARP回应包是如何处理的？这里需要特别关注ARP请求包的内容，在上面的图解里面，ARP请求包的完整信息是：我的IP地址是IP1，MAC地址是MAC1，请问谁是PC2，你的IP2对应的MAC地址是多少？

简单来说，**ARP请求首先有"自我介绍"，然后才是询问**。这样的话，PC2在收到请求之后，就可以将PC1的IP和MAC映射信息存储在本地的【ARP缓存表】，既然知道PC1在哪里，就可以返回ARP单播回应包。

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-2485a31a69987ced88931c4a876bc4b3_720w.jpg)

这张图我们需要得到两个信息：①被询问者PC2先生成了ARP映射信息，然后才是询问者PC1；②PC3和PC4等其他主机，无法收到这个ARP回应包，因为是单播形式。

**小结：**ARP协议通过"一问一答"实现交互，但是"问"和"答"都有讲究，**"问"是通过广播形式实现，"答"是通过单播形式**。



## ARP数据包解读

为了让大家更好的理解ARP协议以及广播和单播的概念，我们来看一下用Wireshark抓取到的真实网络中的ARP过程，通过数据包的方式来呈现，地址信息如下，部分MAC信息隐去。（建议初学者用GNS3配合Wireshark来抓取协议包进行分析，相比真实网络更加干净，方便分析）

主机1 <---> 主机2

主机1： IP1 10.1.20.64 MAC1：00:08:ca:xx:xx:xx

主机2： IP2 10.1.20.109 MAC2：44:6d:57:xx:xx:xx



**【ARP请求包】**

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-5e6176abaacf3839113a116891df9bde_720w.jpg)



**【ARP回应包】**

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-2216de823ce85a2cb629d2a0c774f9c3_720w.jpg)



**【ARP协议字段解读】**

Hardware type ：硬件类型，标识链路层协议

Protocol type： 协议类型，标识网络层协议

Hardware size ：硬件地址大小，标识MAC地址长度，这里是6个字节（48bti）

Protocol size： 协议地址大小，标识IP地址长度，这里是4个字节（32bit）

Opcode： 操作代码，标识ARP数据包类型，1表示请求，2表示回应

Sender MAC address ：发送者MAC

Sender IP address ：发送者IP

Target MAC address ：目标MAC，此处全0表示在请求

Target IP address： 目标IP



## ARP攻击原理

但凡局域网存在ARP攻击，都说明网络存在"中间人"，我们可以用下图来解释。

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-816508f970cea1daf14f0610569c53db_720w.jpg)

在这个局域网里面，PC1、PC2、PC3三台主机共同连接到交换机SW1上面，对应3个接口port1/2/3。假设PC3这台主机安装了ARP攻击软件或遭受ARP病毒，成为这个网络的攻击者（hacker），接下来，PC3是如何攻击的？先不急，先来回顾下PC1和PC2是如何通信的。

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-6522b0e3e1b7e7058152e70953428c74_720w.jpg)

①PC1需要跟PC2通信，通过ARP请求包询问PC2的MAC地址，由于采用广播形式，所以交换机将ARP请求包从接口P1广播到P2和PC3。（注：**交换机收到广播/组播/未知帧都会其他接口泛洪**）



②PC2根据询问信息，返回ARP单播回应包；此时PC3作为攻击者，没有返回ARP包，但是处于"**监听**"状态，为后续攻击做准备。



③PC1和PC2根据ARP问答，将各自的ARP映射信息（IP-MAC）存储在本地ARP缓存表。



④交换机根据其学习机制，记录MAC地址对应的接口信息，存储在**CAM缓存表**（也称为MAC地址表）。交换机收到数据包时，会解封装数据包，根据**目标MAC**字段进行转发。



关于上面的图解，我们要记住这些关键知识（敲黑板！）：

①主机通信需要查找ARP表，而交换机通信需要查找CAM表（路由器则查找Route表）。

注：ARP表：ip<->mac CAM表：mac<->port （Route表：route<->port）

②交换机基于源MAC地址学习，基于目的MAC地址转发。

③同一局域网内，攻击者可以根据主机的ARP广播请求监听其IP和MAC信息。

注：这里是"被动监听"，跟后面要谈到的"主动扫描"，原理上有区分，这里先埋个坑）



接下来是重点，我们来看看PC3（Hacker）是如何发起ARP攻击的=>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-383f86e253b689d40ff20d648ce7afac_720w.jpg)

正常情况下，若收到的ARP请求不是给自己的，则直接丢弃；而这里PC3（Hacker）在监听之后，发起了ARP回应包：**我就是PC2（IP2-MAC3）**。

从拓扑可以出现，PC3明明是IP3对应MAC3，很显然这就是一个ARP欺骗行为。于此同时，PC2正常的ARP回应包也交到了PC1手中，我们来看PC1接下来如何处理的：

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-d234cb3ec99e072f16c7000358284641_720w.jpg)



PC1收到两个ARP回应包，内容分别如下：

③我是PC2，我的IP地址是**IP2**，我的MAC地址是**MAC2**；

③我是PC2，我的IP地址是**IP2**，我的MAC地址是**MAC3**；

PC1一脸懵：**咋回事？还有这操作？不管了，我选最新的！（后到优先）**

这里给大家顺便普及下网络协议里各种表在处理缓存信息的方式：

要么"先到先得"，要么"后到优先"。上面提到的ARP和CAM表，就是遵循"后到优先"原则



那么问题来了，上面两个ARP回应包到底哪个先到哪个后到呢？

作为初学者，可能还在纠结前后这种naive的问题；而作为hacker，只要持续不停发出ARP欺骗包，就一定能够覆盖掉正常的ARP回应包。**稳健的ARP嗅探/渗透工具，能在短时间内高并发做网络扫描（例如1秒钟成千上百的数据包），能够持续对外发送欺骗包。**



无论如何，当PC1和PC2这种"小白"用户遇到PC3（hacker）时，最终的结果一定是这样的：

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-f898793fd3a4f37272dfc31d2ecbaa55_720w.jpg)

小白 vs 黑客，很明显的较量，PC1最终记录的是虚假的ARP映射：IP2<->MAC3，得到错误信息的PC1，接下来会发生什么情况呢？（我们以PC1 ping PC2为例）

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-bade17b91e12c9f9ad5536858abf1350_720w.jpg)



根据数据封装规则，当PC1要跟PC2进行通信时，无论是发生PING包还是发送其他数据，首先要查找ARP表，然后在网络层打上源目IP，在链路层打上源目MAC，然后将数据包发送给交换机。交换机收到之后对数据进行解封装，并且查看CAM表（基于目的MAC转发），由于目标MAC3对应Port3，所以交换机自然而然将其转发给PC3。



就这样，PC1本来要发给PC2的数据包，落到了PC3（Hacker）手里，这就完成了一次完整的ARP攻击。反过来，**如果PC2要将数据包发送给PC1，PC3仍然可以以同样的ARP欺骗实现攻击**，这就有了下面这张图（PC3既欺骗了PC1，也欺骗了PC2）。

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-317858c5badea87786011cb8eda6d3fd_720w.jpg)

此时，PC1和PC2的通信数据流被PC3拦截，形成了典型的"**中间人攻击**"。那么，一旦被攻击并拦截，攻击者能做什么，普通用户又会遭受什么损失？这里给大家举几个常见的例子=>



①攻击者既然操控了数据流，那么直接断开通信是轻而易举的，即**"断网攻击"**，例如，PC1发给PC2的数据在PC3这里可以直接丢弃，而如果这里的PC2是一台出口路由器（无线路由器），那就意味着PC1直接无法连上互联网。



②"断网攻击"显然容易被发现，而且比较"残忍"，所以就有了更加常见的应用-**"限速"**。例如，在宿舍上网突然很慢，在网吧上网突然打不开网页，如果这个网络没有安全防御，那么很有可能有"内鬼"。



③其实无论是"断网攻击"还是"限速"，整体还是比较"善良"，因为这里流量里面的核心数据还没有被"提取"出来。如果攻击者是一名真正的黑客，他的目的一定不会这么无聊，因为内网流量对于黑客是没有太大价值的，而只有**"用户隐私"**，例如常见网站的登录账号密码，这些才是最有价值的。



**问：遭受ARP攻击之后，哪些账号可能被窃取？**

答：**任何基于明文传输的应用，都可以被窃取。**例如，如果一个网站不是HTTPS协议，而是基于HTTP明文传输，那么当你登录这个网站时，你的密码就会被窃取。除了http（web应用），常见的还有telnet、ftp、pop3/smtp/imap（邮箱）等应用，都很容易泄露密码。



### 常见ARP渗透工具与底层原理分析

基于ARP欺骗原理设计出来的渗透/攻击工具非常多，而最终能实现什么功能则各有差异，简单举几个例子：



①无毒无害型的**仅具备ARP扫描**功能，用来发现内网主机；例如Metasploit里面的arping/arpscan相关模块；



②**ARP扫描+流量控制**（限速或限制能上哪些网站和应用）；例如Windows下的P2P终结者；



③**ARP扫描+账号窃取**（网站、邮箱、各种）；最强的莫过于Windows下的Cain，当然还有跨平台的Ettercap（需配合其他工具）；



当然，如果攻击者足够强悍，也可以基于协议底层原理，编写自己的ARP工具。这里我**通过wirehshark给大家还原真实网络中常见的ARP扫描和欺骗攻击**（具体的软件使用这里暂时不出现，大家重点关注底层实现）



![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-af057558ef563955ede46f22ea77c335_720w.jpg)



在这张图里面，Hacker（就是我...）接入了一个WiFi网络，这个10.1.20.0/24便是所在的网段。刚进来一个陌生网络，Hacker只知道自己的IP信息，例如IP地址是10.1.20.253，网关地址是10.1.20.254，而这个局域网的其他设备是什么？有多少台？地址分布是多少？Hacker都不知道，接下来怎么办呢？是不是要直接发动ARP攻击了？



不用这么着急，咋们至少要先了解下这个网络，进行基本的**扫描和踩点**。这个时候通过ARP工具对这个WiFi网络进行扫描，具体的数据包截图如下：

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-bc0c22f0043748b52cceaf185804694b_720w.jpg)



上面的ARP扫描过程，大概的情况是这样的=>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-4be85864ff433d20b9e1165fe7cc36c3_720w.jpg)

其实，这就是典型的"盲扫"或者"暴力扫描"：反正我不知道网络到底有多少主机，那我就尝试一下把整个网段全部问一遍得了。**好比老师上课点名，把每个学生的桌位号念一遍，谁举手就到勤，没举手就算逃课。**



那么，这个实际网络里面，到底谁"举手"了呢？我们来看Wireshark抓包情况。



![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-0f25c1953baf27d3770c4267ee5d5972_720w.jpg)



在ARP应答信息里面，除了IP地址和MAC信息，我们还能看到相关的设备厂商信息，例如cisco、meizu、apple、xiaomi等，这其实就是依靠MAC地址前面24位的**OUI（机构唯一标识符）**来识别的。



Wireshark或扫描器能够帮我们将OUI转为对应的厂商（还有一些扫描器基于Netbios协议，还能找到电脑的主机名），所以，扫描之后可以得到下面这张图片=>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-c2b83eee5c96827c133c45a25d570c56_720w.jpg)

通过扫描，我们已经知道了整个网络的主机信息，例如20.254对应cisco，应该是路由器，20.248对应apple，是苹果手机，20.249对应xiaomi，是小米手机，以此类推.....



接下来，如何进行ARP欺骗攻击呢？这里将最重点的数据包截取出来=>

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-2788e3e061f7ee8af394dea99a5e732c_720w.jpg)



根据之前的信息，我们知道00:08:ca:86:f8:0f其实就是hacker的mac地址，并且对应的真正的IP地址应该是10.1.20.253。而这里很明显是**hacker在欺骗局域网其他主机，它对外声称：自己就是"所有人"**。尤其是上面标红的主机，我们已经知道是小米、思科、苹果等设备，但是hacker都声明是自己！这样做的意义在于覆盖掉其他主机的ARP缓存表信息，并生成错误的ARP映射，最终将通信流量交给hacker。



当然，还有另外一种ARP欺骗的做法：**hacker告诉所有人，自己就是网关**。因为其他主机访问互联网必经之路便是网关（出口路由器/无线路由器），通过这种方式，同样可以截取到用户数据流，这里给出另外一个网络的实现过程=>



**Hacker欺骗主机Honhai，告诉它：我就是网关（10.1.1.254）**

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-412742914a0c84bb53a020e66a771ecb_720w.jpg)



**Hacker欺骗主机Apple，告诉它：我就是网关（10.1.1.254）**

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/v2-1185c84e361ae6e5be13a78d917cec20_720w.jpg)



依此类推，Hacker会告诉局域网所有主机：自己就是网关，并且后续可以把数据都丢给我，我来转发到互联网。



**总结**

①ARP缓存表基于"后到优先"原则，IP与MAC的映射信息能被覆盖；

②ARP攻击基于伪造的ARP回应包，黑客通过构造"错位"的IP和MAC映射，覆盖主机的ARP表（也被称为"ARP毒化"），最终截取用户的数据流；

③一旦遭受ARP攻击，账号密码都可能被窃取（如果通信协议不是加密的）；

④通过Wireshark数据包分析，我们掌握了真实网络中ARP底层攻击原理及数据包组成。



https://zhuanlan.zhihu.com/p/28771785



https://zhuanlan.zhihu.com/p/28818627