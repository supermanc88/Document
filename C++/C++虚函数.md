
在编译器中打开内存布局：

C/C++ -> 命令行 在其他选项上填写 `/d1 reportAllClassLayout`


C++的虚函数主要作用是“运行时多态”，父类中提供虚函数的实现，为子类提供默认的函数实现。

子类可以重写父类的虚函数实现子类的特殊化。

例：
```
class A
{
	int a;
	int b;
public:
	virtual void Say()
	{
		printf("A Say\n");
	}
	virtual void Say2()
	{
		printf("A Say2\n");
	}

	void Say3()
	{
		printf("A Say3\n");
	}
};

class B : public A
{
	int c;
public:
	void Say()
	{
		printf("B Say\n");
	}
	void Say3()
	{
		printf("B Say3\n");
	}
};

class C : public A
{
	int d;
public:
	virtual void Say()
	{
		printf("C Say\n");
	}
};

class D : public B, public C
{
	int e;
public:
	virtual  void Say()
	{
		printf("D Say\n");
	}
};
```

内存布局如下：

```
class A	size(12):
	+---
 0	| {vfptr}
 4	| a
 8	| b
	+---

A::$vftable@:
	| &A_meta
	|  0
 0	| &A::Say
 1	| &A::Say2

A::Say this adjustor: 0
A::Say2 this adjustor: 0
```

先看类A，虚表指针放在第一位（类内偏移从0开始），后面依次是成员变量，成员变量依据声明的顺序进行排列，成员函数不占内存空间。

紧接着是虚表在内存中的分布，下面列出了虚函数，左侧的0和1是这个虚函数的序号,如果还有更多的虚函数，会依次列出来。

编译器是在构造函数中创建这个虚表指针和虚表的。

那么编译器是如何利用虚表指针与虚表来实现多态的呢？是这样的，**<span style="color:red">当创建一个含有虚函数的父类的对象时，编译器在对象构造时将虚表指针指向父类的虚函数；同样，当创建子类的对象时，编译器在构造函数里将虚表指针（子类只有一个虚表指针，它来自父类）指向子类的虚表（这个虚表里面的虚函数入口地址是子类的）。<span>**

所以，如果是调用`A *a = new B();`,生成的是子类的对象，在构造时，子类对象的虚指针指向的是子类的虚表，接着由`B*`到`A*`的转换并没有改变虚表指针，所以这时候`a->Say()`实际上是`b->Say()`，它在构造的时候就已经指向了子类的`Say`，所以调用的是子类的虚函数，这就是多态了。

```
class B	size(16):
	+---
 0	| +--- (base class A)
 0	| | {vfptr}
 4	| | a
 8	| | b
	| +---
12	| c
	+---

B::$vftable@:
	| &B_meta
	|  0
 0	| &B::Say
 1	| &A::Say2

B::Say this adjustor: 0
```

看类B，可以看到，虚表指针被继承了，且仍位于内存排布的起始处，下面是父类的成员变量a和b，接下来是子类的成员变量c，注意虚表指针只有一个，子类并没有再生成虚表指针了。

接下来看一下虚表，我们重写了父类的`Say`虚函数，虚表的0号是子类的`Say`，而1号是从父类继承下来的`Say2`。也就是说，如果定义了B的对象，那么在构造是，虚表指针就会指向这个虚表，以后如果是调用Say，那么会从子类中寻找对应的虚函数，如果是调用Say2，那么会从父类中寻找对应的虚函数。

下面看一下以下代码的运行结果：

```
int main()
{
	B * pb = (B*)new A();           // 虚表指针指向A的虚表
	pb->Say();                      // A Say
	pb->Say2();                     // A Say2
	pb->Say3();                     // Say3是普通函数，经过转换，Say3其实是B类的函数

	A * pa = (A*)new B();
	pa->Say();
	pa->Say2();
	pa->Say3();
}
```

多重继承内存布局：

```
class D	size(36):
	+---
 0	| +--- (base class B)
 0	| | +--- (base class A)
 0	| | | {vfptr}
 4	| | | a
 8	| | | b
	| | +---
12	| | c
	| +---
16	| +--- (base class C)
16	| | +--- (base class A)
16	| | | {vfptr}
20	| | | a
24	| | | b
	| | +---
28	| | d
	| +---
32	| e
	+---

D::$vftable@B@:
	| &D_meta
	|  0
 0	| &D::Say
 1	| &A::Say2

D::$vftable@C@:
	| -16
 0	| &thunk: this-=16; goto D::Say
 1	| &A::Say2

D::Say this adjustor: 0
```

多重继承中含有多个虚表指针，此示例中包含两个虚表指针。

本文暂不讨论虚继承。