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

