# Ubuntu安装rime输入法

## 安装ibus

```
sudo apt-get install ibus-rime
```

## 安装Plum

可通过Plum安装自己喜欢的输入法：

```
curl -fsSL https://git.io/rime-install | bash

bash rime-install wubi pinyin-simp
```

## 配置rime

![setting](./images/2019-11-17_00-49-36.png)

在`default.yaml`中添加上输入法

![setting](./images/2019-11-17_00-52-01.png)


## 添加输入源

![inputsource](./images/2019-11-17_00-37-06.png)

重启操作系统就可以用了


## 安装emoji

### Linux 安装

使用Plum安装：

```
bash rime-install emoji
```

配置：

```
bash rime-install emoji:customize:schema=luna_pinyin
```

### Windows安装

![windowssetupemoji](./images/1575944204(1).jpg)

![windowssetupemoji1](./images/1575944294(1).jpg)

在windows上使用官方方法不容易成功，现使用手动方式：

![manualsetup](./images/1575950308(1).jpg)

在安装emoji后，用户文件夹中会出现`emoji_suggestion.yaml`和`opencc`文件夹。

比如我要给五笔添加emoji，新建文件`wubi86.custom.yaml`

将`emoji_suggestion.yaml`中的内容全部复制到`wubi86.custom.yaml`中，重新部署即可。


