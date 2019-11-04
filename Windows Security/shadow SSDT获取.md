# Shadow SSDT获取

本文仅提供思路

## x86 系统
### 硬编码获取

已知`KeServiceDescriptorTable`是导出的，个别系统可以通过此变量的地址加偏移找出`KeServiceDescriptorTableShadow`的地址。

例：
```
// win7 32位下
kd> dps nt!KeServiceDescriptorTable
83fbc9c0  83ed0d9c nt!KiServiceTable
83fbc9c4  00000000
83fbc9c8  00000191
83fbc9cc  83ed13e4 nt!KiArgumentTable
83fbc9d0  00000000
83fbc9d4  00000000
83fbc9d8  00000000
83fbc9dc  00000000
83fbc9e0  83f2f6af nt!KdpSwitchProcessor
83fbc9e4  00000000
83fbc9e8  02f7d1be
83fbc9ec  000000bb
83fbc9f0  00000011
83fbc9f4  00000100
83fbc9f8  5385d2ba
83fbc9fc  d717548f
83fbca00  83ed0d9c nt!KiServiceTable
83fbca04  00000000
83fbca08  00000191
83fbca0c  83ed13e4 nt!KiArgumentTable
83fbca10  926a6000 win32k!W32pServiceTable
83fbca14  00000000
83fbca18  00000339
83fbca1c  926a702c win32k!W32pArgumentTable
83fbca20  00000000
83fbca24  00000000
83fbca28  83fbca24 nt!KiNonNumaDistance
83fbca2c  00000340
83fbca30  00000340
83fbca34  857d5488
83fbca38  00000007
83fbca3c  00000000

```


### 通过KeAddSystemServiceTable获取

这个方法更加通用
```
kd> u nt!KeAddSystemServiceTable L20
nt!KeAddSystemServiceTable:
81588690 8bff            mov     edi,edi
81588692 55              push    ebp
81588693 8bec            mov     ebp,esp
81588695 837d1801        cmp     dword ptr [ebp+18h],1
81588699 7750            ja      nt!KeAddSystemServiceTable+0x5b (815886eb)
8158869b 8b4d18          mov     ecx,dword ptr [ebp+18h]
8158869e c1e104          shl     ecx,4
815886a1 83b90024438100  cmp     dword ptr nt!KeServiceDescriptorTable (81432400)[ecx],0
815886a8 7541            jne     nt!KeAddSystemServiceTable+0x5b (815886eb)
815886aa 83b9c023438100  cmp     dword ptr nt!KeServiceDescriptorTableShadow (814323c0)[ecx],0
815886b1 7538            jne     nt!KeAddSystemServiceTable+0x5b (815886eb)
815886b3 837d1801        cmp     dword ptr [ebp+18h],1
815886b7 8b5508          mov     edx,dword ptr [ebp+8]
815886ba 8b4514          mov     eax,dword ptr [ebp+14h]
815886bd 56              push    esi
815886be 8b750c          mov     esi,dword ptr [ebp+0Ch]
815886c1 57              push    edi
815886c2 8b7d10          mov     edi,dword ptr [ebp+10h]
815886c5 8991c0234381    mov     dword ptr nt!KeServiceDescriptorTableShadow (814323c0)[ecx],edx
815886cb 89b1c4234381    mov     dword ptr nt!KeServiceDescriptorTableShadow+0x4 (814323c4)[ecx],esi
815886d1 89b9c8234381    mov     dword ptr nt!KeServiceDescriptorTableShadow+0x8 (814323c8)[ecx],edi
815886d7 8981cc234381    mov     dword ptr nt!KeServiceDescriptorTableShadow+0xc (814323cc)[ecx],eax
815886dd 0f85cd240d00    jne     nt! ?? ::NNGAKEGL::`string'+0x25e0a (8165abb0)
815886e3 5f              pop     edi
815886e4 b001            mov     al,1
815886e6 5e              pop     esi
815886e7 5d              pop     ebp
815886e8 c21400          ret     14h
815886eb 32c0            xor     al,al
815886ed ebf8            jmp     nt!KeAddSystemServiceTable+0x57 (815886e7)
815886ef cc              int     3
815886f0 cc              int     3

```

在这一句
```
815886c5 8991c0234381    mov     dword ptr nt!KeServiceDescriptorTableShadow (814323c0)[ecx]
```
可以直接找到`KeServiceDescriptorTableShadow`的地址，通过特征码搜索即可。


## x64 系统

在64位系统下最好还是老老实实的搜索特征码：

### 硬编码

不是说硬编码彻底不能用：
```
// win7 x64下还是可以找到
kd> dqs nt!KeServiceDescriptorTable
fffff800`04103840  fffff800`03ed3300 nt!KiServiceTable
fffff800`04103848  00000000`00000000
fffff800`04103850  00000000`00000191
fffff800`04103858  fffff800`03ed3f8c nt!KiArgumentTable
fffff800`04103860  00000000`00000000
fffff800`04103868  00000000`00000000
fffff800`04103870  00000000`00000000
fffff800`04103878  00000000`00000000
fffff800`04103880  fffff800`03ed3300 nt!KiServiceTable
fffff800`04103888  00000000`00000000
fffff800`04103890  00000000`00000191
fffff800`04103898  fffff800`03ed3f8c nt!KiArgumentTable
fffff800`041038a0  fffff960`00171f00 win32k!W32pServiceTable
fffff800`041038a8  00000000`00000000
fffff800`041038b0  00000000`0000033b
fffff800`041038b8  fffff960`00173c1c win32k!W32pArgumentTable
```

### 特征码搜索

x64下不使用`KeAddSystemServiceTable`函数搜索。使用`KiSystemCall64`进行搜索

`KiSystemCall64`的地址通过`__readmsr(0xc0000082)`获取

```
kd> u nt!KiSystemCall64 L100
nt!KiSystemCall64:
fffff800`03ed1640 0f01f8          swapgs
fffff800`03ed1643 654889242510000000 mov   qword ptr gs:[10h],rsp
fffff800`03ed164c 65488b2425a8010000 mov   rsp,qword ptr gs:[1A8h]
fffff800`03ed1655 6a2b            push    2Bh
fffff800`03ed1657 65ff342510000000 push    qword ptr g
...
fffff800`03ed1765 8bf8            mov     edi,eax
fffff800`03ed1767 c1ef07          shr     edi,7
fffff800`03ed176a 83e720          and     edi,20h
fffff800`03ed176d 25ff0f0000      and     eax,0FFFh
nt!KiSystemServiceRepeat:
fffff800`03ed1772 4c8d15c7202300  lea     r10,[nt!KeServiceDescriptorTable (fffff800`04103840)]
fffff800`03ed1779 4c8d1d00212300  lea     r11,[nt!KeServiceDescriptorTableShadow (fffff800`04103880)]
fffff800`03ed1780 f7830001000080000000 test dword ptr [rbx+100h],80h
fffff800`03ed178a 4d0f45d3        cmovne  r10,r11
fffff800`03ed178e 423b441710      cmp     eax,dword ptr [rdi+r10+10h]
fffff800`03ed1793 0f83e9020000    jae     nt!KiSystemServiceExit+0x1a7 (fffff800`03ed1a82)
fffff800`03ed1799 4e8b1417        mov     r10,qword ptr [rdi+r10]
fffff800`03ed179d 4d631c82        movsxd  r11,dword ptr [r10+rax*4]
fffff800`03ed17a1 498bc3          mov     rax,r11
...
```


在win10下，在`KiSystemCall64`函数中是找不到的，这时需要通过特征码先在此函数中找到`KiSystemServiceUser`函数地址

```
fffff803`23733383 e9631ae9ff      jmp     nt!KiSystemServiceUser(fffff803`235c4deb)
fffff803`23733388 c3              ret
```

然后在`KiSystemServiceUser`函数中通过特征码搜索出`Shadow SSDT`的地址。
