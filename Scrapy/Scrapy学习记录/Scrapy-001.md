# Scrapy-001

Scrapy版本：
```
superman@debian:~/testscrapy$ scrapy version
Scrapy 1.5.1
```

操作系统：
```
superman@debian:~/testscrapy$ uname -a
Linux debian 4.9.0-7-686 #1 SMP Debian 4.9.110-3+deb9u2 (2018-08-13) i686 GNU/Linux
```

## 打开一个网页

```
scrapy shell https://www.baidu.com
```
通过此命令可以打开一个并返回一些输出。现在可以在python提示符下，用它来调试刚才加载的页面，`ctrl+d`退出。

## 生成一个项目
```
superman@debian:~$ scrapy startproject test1
New Scrapy project 'test1', using template directory '/usr/local/lib/python2.7/dist-packages/scrapy/templates/project', created in:
    /home/superman/test1

You can start your first spider with:
    cd test1
    scrapy genspider example example.com

superman@debian:~$ cd test1
superman@debian:~/test1$ tree
.
├── scrapy.cfg
└── test1
    ├── __init__.py
    ├── items.py
    ├── middlewares.py
    ├── pipelines.py
    ├── settings.py
    └── spiders
        └── __init__.py

2 directories, 7 files

```
test1为将要创建的项目名。
