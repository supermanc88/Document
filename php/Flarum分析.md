# Flarum源码分析

## 安装

环境依赖：

- **Apache** (with mod_rewrite enabled) or **Nginx**
- **PHP 7.3+** with the following extensions: curl, dom, gd, json, mbstring, openssl, pdo_mysql, tokenizer, zip
- **MySQL 5.6+** or **MariaDB 10.0.5+**
- **SSH (command-line) access** to run Composer



安装命令：

```shell
composer create-project flarum/flarum .
```



## 目录结构

```shell
[root@VM-4-16-centos www.xxx.com]# tree -L 1
.
|-- CHANGELOG.md
|-- composer.json
|-- composer.lock
|-- config.php
|-- extend.php
|-- flarum
|-- LICENSE
|-- public
|-- README.md
|-- site.php
|-- storage
`-- vendor

3 directories, 9 files
```



### composer.json

`composer.json`文件内容：

```json
{
    "name": "flarum/flarum",
    "description": "Delightfully simple forum software.",
    "type": "project",
    "keywords": [
        "forum",
        "discussion"
    ],
    "homepage": "https://flarum.org/",
    "license": "MIT",
    "authors": [
        {
            "name": "Flarum",
            "email": "info@flarum.org",
            "homepage": "https://flarum.org/team"
        }
    ],
    "support": {
        "issues": "https://github.com/flarum/core/issues",
        "source": "https://github.com/flarum/flarum",
        "docs": "https://flarum.org/docs/"
    },
    "require": {
        "flarum-lang/chinese-simplified": "^1.0",
        "flarum/approval": "*",
        "flarum/bbcode": "*",
        "flarum/core": "^1.0",
        "flarum/emoji": "*",
        "flarum/flags": "*",
        "flarum/lang-english": "*",
        "flarum/likes": "*",
        "flarum/lock": "*",
        "flarum/markdown": "*",
        "flarum/mentions": "*",
        "flarum/nicknames": "*",
        "flarum/pusher": "*",
        "flarum/statistics": "*",
        "flarum/sticky": "*",
        "flarum/subscriptions": "*",
        "flarum/suspend": "*",
        "flarum/tags": "*"
    },
    "config": {
        "preferred-install": "dist",
        "sort-packages": true
    }
}
```



所有的安装及依赖均存放在`vendor`目录下。