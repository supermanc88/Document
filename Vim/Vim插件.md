记录一些常用的好用的Vim插件

## 插件管理：
[junegunn/vim-plug](https://github.com/junegunn/vim-plug)

特点是多线程下载，速度快

## 代码补全
[Valloric/YouCompleteMe](https://github.com/Valloric/YouCompleteMe)

安装环境：
```
python 要和 Vim编译选项的版本一致  :version
CMake 最新版本

// Windows 下要安装 Visual Studio 15 或 17
// Linux 下直接运行下面命令即可
```
编译命令：
```
python install.py --clang-completer --msvc 15
```