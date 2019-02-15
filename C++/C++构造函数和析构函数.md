## 类的构造函数

### 默认的构造函数

类的构造函数是类的一种特殊的成员函数，它会在每次创建类的新对象时执行

构造函数的名称和类的名称是完全相同的，并且不会返回任何类型，也不会返回`void`。构造函数可用于为某些成员变量设置初始值。

```
class Base
{
public:
	Base();
};

Base::Base()
{
	printf("这里是Base类的构造函数\n");
}

int main()
{
    Base b;
    return 0;
}
```
上述代码执行时会打印构造函数里的语句。

### 带参数的构造函数

默认的构造函数没有任何参数，但如果需要，构造函数也可以带有参数。这样在创建对象时就会给对象赋初始值。

```
class Base
{
public:
	Base(int x);
	void PrintA();
private:
	int a;
};

Base::Base(int x)
{
	a = x;
	printf("这里是Base类的构造函数\n");
}

void Base::PrintA()
{
	printf("a的值为%d\n", a);
}

int main()
{
    Base b(5);
    b.PrintA();
    return 0;
}
```

### 使用初始化列表来初始化字段

```
class Base
{
public:
	Base(int x, int y);
	void PrintA();
private:
	int a;
	int b;
};

Base::Base(int x, int y):a(x),b(y)
{
    // 上面的初始化列表和 a = x; b = y; 等同
	printf("这里是Base类的构造函数\n");
}

void Base::PrintA()
{
	printf("a=%d,b=%d\n", a,b);
}
```

## 析构函数

```
class Base
{
public:
	Base(int x, int y);
	void PrintA();
	~Base()
	{
		printf("这里是Base类的析构函数\n");
	}
private:
	int a;
	int b;
};
```
类的析构函数会在每次删除所创建的对象时执行。