## 什么是UWP

全称为：Universal Windows Platform（通用Windows平台）

## UWP有什么特点

- 安全：UWP应用声明它访问哪些设备和资源，用户必须给予访问授权

- 可以让UI适应不同的设备屏幕尺寸、分辨率和DPI

- 由Microsoft Store提供

- 可以安全安装、升级和卸载，不对计算机造成影响（卸载残留或软件风险等）

- 通知和互动更好看

## AppContainer

所有的UWP程序均运行在`AppContainer`中。

隔离是AppContainer执行环境的首要目标。通过将应用程序与不需要的资源和其他应用程序隔离，可以最大限度地减少恶意操作的机会。授予基于最小特权访问防止应用程序和用户访问的资源超出了他们的权利。控制对资源的访问可以保护进程，设备和网络。

### 凭证隔离

管理身份和凭证，AppContainer阻止使用用户的凭据来获取资源或登录到其他环境。AppContainer环境创建一个用户和应用程序的组合的身份，因此凭证是唯一的`用户/应用`配对和应用程序不能冒充用户的标识符。

### 设备隔离

隔离设备资源，AppContainer环境阻止应用程序恶意利用这些设备。这些设备是默认阻止的，需要使用必须授权。

### 文件隔离

AppContainer控制文件和注册表的访问。读写权限可授予特定的文件和注册表项。

### 网络隔离

AppContainer将应用程序与网络资源隔离开来，防止应用程序逃离这个环境并恶意利用网络资源。

AppContainer 禁用了 `Loopback`, 故UWP不能访问127.0.0.1

### 进程隔离

通过沙箱化应用程序内核对象，AppContainer环境可以防止应用程序影响或受其他应用程序进程的影响

### 窗口隔离

AppContainer环境将应用程序与其他窗口隔离，可防止应用程序影响其他应用程序界面

## 查找对应的UWP容器名和SID

```Shell
// 打开注册表
计算机\HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppContainer\Mappings
```

左侧为SID，`Moniker`为容器名

Windows提供`CheckNetIsolation.exe`应用程序控制Loopback

## Github开源项目

[`tiagonmas/Windows-Loopback-Exemption-Manager`](https://github.com/tiagonmas/Windows-Loopback-Exemption-Manager)