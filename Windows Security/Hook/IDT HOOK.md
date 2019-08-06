## 什么是IDT

IDT (Interrupt Descriptor Table) 中断描述表

IDT表中存储了ISR(Interrupt Service Routines) 中断服务程序，这些例程会在中断产生时被调用。

IDT是一个有256个入口的线性表，每个IDT的入口是个8字节的描述符，所以整个IDT表的大小为`256 * 8 = 2048 bytes`，**每个中断向量关联了一个中断处理过程。所谓的中断向量就是把每个中断或者异常用一个0-255的数字识别。Intel称这个数字为向量。**

对于IDT，操作系统使用IDTR寄存器来记录IDT的位置和大小。

`IDTR`寄存器是`48`位寄存器，用于保存IDT的信息。其中`低16位`代表IDT的大小，如下为`0x7ff`，`高32位`代表IDT的基地址，如下为`0x80b95400`。

**可以使用指令`sidt`读出IDTR寄存器中的信息，从而找到IDT在内存中的位置。**

```C++
   typedef struct
   {
   WORD IDTLimit;
   WORD LowIDTbase;   //IDT的低半地址
   WORD HiIDTbase;     //IDT的高半地址
   } IDTINFO;
```

```C++
	IDTINFO idt_info; 
	  __asm{

         sidt idt_info;          //获取IDTINFO
 
      }
```


首先看一下`IDTR`和`IDTL`寄存器的值

```cmd
kd> r idtr
idtr=80b95400
kd> r idtr, idtl
idtr=80b95400 idtl=000007ff
```

## IDT HOOK

IDT,中断就是停下现在的活动，去完成新的任务。一个中断可以源于软件或硬件。比如出现页错误，调用IDT中的0x0E。或用户请求系统服务（SSDT）时，调用IDT中的0x2E。而系统服务的调用是经常的，这个中断就能触发。我们现在就想办法，先在系统中找到IDT，然后确定0x2E在IDT中的地址，最后用我们的函数地址去取代它，这样一来，用户的进程一调用SSDT，我们的HOOK函数即被激发。

```C++
#pragma pack(1)
typedef struct
{
   WORD LowOffset;            //入口的低半地址
   WORD selector;
   BYTE unused_lo;
   unsigned char unused_hi:5;      // stored TYPE ?
   unsigned char DPL:2;
   unsigned char P:1;          // vector is present
   WORD HiOffset;           //入口地址的低半地址
} IDTENTRY;
#pragma pack()
```

```C++
 int HookInterrupts()
   {
 
      IDTINFO idt_info;           //SIDT将返回的结构
 
      IDTENTRY* idt_entries;     //IDT的所有入口
 
      IDTENTRY* int2e_entry;     //我们目标的入口
 
      __asm{
 
         sidt idt_info;          //获取IDTINFO
 
      }
     //获取所有的入口
      idt_entries =
 
     (IDTENTRY*)MAKELONG(idt_info.LowIDTbase,idt_info.HiIDTbase);
 
   //保存真实的2e地址
      KiRealSystemServiceISR_Ptr =
                                 MAKELONG(idt_entries[NT_SYSTEM_SERVICE_INT].LowOffset,
 
            idt_entries[NT_SYSTEM_SERVICE_INT].HiOffset);
 
   //获取0x2E的入口地址
      int2e_entry = &(idt_entries[NT_SYSTEM_SERVICE_INT]);
 
      __asm{
 
        cli;                        // 屏蔽中断，防止被打扰
 
        lea eax,MyKiSystemService; // 获得我们hook函数的地址，保存在eax
 
        mov ebx, int2e_entry;       // 0x2E在IDT中的地址
 
        mov [ebx],ax;               // 把我们hook函数的低地址写入低地址
 
      shr eax,16                  //eax右移16，得到高地址
 
        mov [ebx+6],ax;            // 写入高地址
 
      sti;                       //开中断
 
      }
 
      return 0;
 
   }
```

## 键盘中断

8086PC机当中，键盘的输入将会引发9号中断，BIOS提供了int 9的中断例程。CPU在9号中断发生之后，会去执行`int 9`中断例程，然后从`60h端口`当中读取出扫描码，并且将其转换为相应的ASCII码和状态信息，存储在内存的指定的空间(键盘缓冲区或状态字节)当中。

> 一般的键盘输入，在CPU执行完int 9 中断例程之后都放到了键盘缓冲区当中，键盘缓冲区有`16个字单元`，可以存储15个按键扫描码和对应的ASCII码，这里之所以只能放15个是因为键盘缓冲区是用环形队列结构管理的内存区域，虽然缓冲区的本身长度为16个字，但出于判断“对列满”的考虑，它最多只能保存15个键盘信息。

BIOS提供了`int 16h`中断例程供程序员调用。这个例程中包含的一个最重要的功能是从键盘缓冲区中读取一个键盘输入。

```asm
mov ah,0
int 16h
```

结果：ah(扫描码)，al(ascii码)
