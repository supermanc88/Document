# go语言静态库的编译和使用

本文主要介绍go语言静态库的编译和使用方法，以windows平台为例，linux平台步骤一样，具体环境如下：

```shell
>echo %GOPATH%
E:\share\git\go_practice\

>echo %GOROOT%
C:\Go\

>tree /F %GOPATH%\src
卷 work 的文件夹 PATH 列表
卷序列号为 0009-D8C8
E:\SHARE\GIT\GO_PRACTICE\SRC
│  main.go
│
└─demo
        demo.go
```

在%GOPATH%\src目录，有demo包和使用demo包的应用程序main.go，main.go代码如下：

```go
package main

import "demo"

func main() {
    demo.Demo()
}
```

demo包中的demo.go代码如下：

```go
package demo

import "fmt"

func Demo() {
    fmt.Println("call demo ...")
}
```

由于demo.go是在%GOPATH%\src目录下的一个包，main.go在import该包后，可以直接使用，运行main.go：

```shell
>go run main.go
call demo ...
```



现在，需要将demo.go编译成静态库demo.a，不提供demo.go的源代码，让main.go也能正常编译运行，详细步骤如下：

### 编译静态库demo.a

```shell
>go install demo
```

在命令行运行go install demo命令，会在%GOPATH%目录下生相应的静态库文件demo.a（windows平台一般在%GOPATH%\src\pkg\windows_amd64目录）。

### 编译main.go

进入main.go所在目录，编译main.go：

```shell
>go tool compile -I E:\share\git\go_practice\pkg\windows_amd64 main.go
```

`-I`**选项指定了demo包的安装路径**，供main.go导入使用，即E:\share\git\go_practice\pkg\win
dows_amd64目录，编译成功后会生成相应的目标文件main.o。



### 链接main.o

```shell
>go tool link -o main.exe -L E:\share\git\go_practice\pkg\windows_amd64 main.o
```

`-L`选项**指定了静态库demo.a的路径**，即E:\share\git\go_practice\pkg\win
dows_amd64目录，链接成功后会生成相应的可执行文件main.exe。



### 运行main.exe

```shell
>main.exe
call demo ...
```



现在，就算把demo目录删除，再次编译链接main.go，也能正确生成main.exe:

```shell
>go tool compile -I E:\share\git\go_practice\pkg\windows_amd64 main.go

>go tool link -o main.exe -L E:\share\git\go_practice\pkg\windows_amd64 main.o

>main.exe
call demo ...
```

但是，如果删除了静态库demo.a，就不能编译main.go，如下：

```shell
>go tool compile -I E:\share\git\go_practice\pkg\windows_amd64 main.go
main.go:3: can't find import: "demo"
```

------





# 发布方案

将要发布的闭源包.a文件放到 $GOROOT/pkg/$GOOS_$GOARCH/ 目录中.

再将发布包的源码保留只有包声明的源文件放到 $GOROOT/src/pkg/ 目录中.

操作如下：

## 发布方：

例如有say包要发布：

```
$cd $GOPATH/src/say
$cat say.go
// say something package
package say

import "fmt"

// private function
func say(str string){
    fmt.Println(str)
}

// Say hi
func Hi(){
    say("Hi......")
}

// Say hello to someone
func Hello(me string){
    say("Hello" + me)
}
```

首先编译生成say包的.a文件(如果要发布到多种系统架构,需要修改编译参数交叉编译出多种发布文件)

```
$go install
$ls $GOPATH/pkg/$GOOS_$GOARCH/say.a
```

其次修改发布包对应的源文件(两种方式任选)

最简单的修改方式

```
$echo 'package say' > say.go
```

保留导出的接口与API注释供查看使用

```
$cat > say.go <<EOF
// say something package
package say

// Say hi
func Hi(){}

// Say hello to someone
func Hello(me string){}
EOF
```

最后发布方提供两份文件:

编译生成say包的.a文件和修改之后的源文件

**say.a**

**say.go**



## 使用方：

复制say.a文件到$GOROOT/pkg/$GOOS_$GOARCH/目录中

复制say.go文件到$GOROOT/src/pkg/say/目录中

```
$cp say.a $GOROOT/pkg/$GOOS_$GOARCH/
$mkdir -p $GOROOT/src/pkg/say/
$cp say.go $GOROOT/src/pkg/say/
```

然后就可以在自己的代码中像使用官方标准库一样使用第三方闭源包了.

<font color='red'>注：这个第三方的路径要和编译时的包路径一致</font>

如果三方包保留了接口与API注释, 还可以直接使用godoc命令查看

```go
$godoc say
PACKAGE DOCUMENTATION

package say
    import "say"

    say something package

FUNCTIONS

func Hi()
    Say hi

func Hello(me string)
    Say hello to someone
```

