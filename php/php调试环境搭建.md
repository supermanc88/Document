# php 调试环境搭建

## 下载xdebug源码编译

```
tar -xzvf xdebug.tgz

cd xdebug

/usr/local/php/bin/phpize

./configure --enable-xdebug --with-php-config=/usr/local/php/bin/php-config

make && make install
```

## 设置php.ini

```
zend_extension = /yourpath/xdebug.so
xdebug.remote_enable = 1
xdebug.remote_autostart= 1
```

## 设置VSCode调试环境

```
"php.executablePath": "/usr/bin/php"
```
