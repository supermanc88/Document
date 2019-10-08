## 安装global

### 卸载原有global

```shell
sudo apt remove global
```

## Leaderf 使用 global功能

```shell
:LeaderfRgRecall 显示上次 rg 的结果
:Leaderf rg 实时检索
:Leaderf rg [option] 后面的 [option] 和 rg 的语法保持一致
```

### 安装最新

需要安装以下信赖库：

```shell
sudo apt-get install libncurses5-dev libncursesw5-dev
```

在[GNU官网](https://www.gnu.org/software/global/download.html)下载最新源码。

```shell
wget http://tamacom.com/global/global-6.6.3.tar.gz

tar -zvxf global-6.6.3.tar.gz

cd global-6.6.3

./configure

make && make install
```

## 安装ripgrep

[github](https://github.com/BurntSushi/ripgrep)

Ubuntu18以下需要下载安装包：

```shell
curl -LO https://github.com/BurntSushi/ripgrep/releases/download/11.0.2/ripgrep_11.0.2_amd64.deb

sudo dpkg -i ripgrep_11.0.2_amd64.deb
```

## 快捷键

```shell
<C-C>, <ESC> : 退出 LeaderF.
<C-R> : 在模糊匹配和正则式匹配之间切换
<C-F> : 在全路径搜索和名字搜索之间切换
<Tab> : 在检索模式和选择模式之间切换
<C-J>, <C-K> : 在结果列表里选择
<C-X> : 在水平窗口打开
<C-]> : 在垂直窗口打开
<C-T> : 在新标签打开
<C-P> : 预览结果
```
