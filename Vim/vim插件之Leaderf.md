## 安装global

### 卸载原有global

```
sudo apt remove global
```

### 安装最新

需要安装以下信赖库：

```
sudo apt-get install libncurses5-dev libncursesw5-dev
```

在[GNU官网](https://www.gnu.org/software/global/download.html)下载最新源码。

```
wget http://tamacom.com/global/global-6.6.3.tar.gz

tar -zvxf global-6.6.3.tar.gz

cd global-6.6.3

./configure

make && make install
```

## 安装ripgrep

[github](https://github.com/BurntSushi/ripgrep)

Ubuntu18以下需要下载安装包：

```
$ curl -LO https://github.com/BurntSushi/ripgrep/releases/download/11.0.2/ripgrep_11.0.2_amd64.deb
$ sudo dpkg -i ripgrep_11.0.2_amd64.deb
```
