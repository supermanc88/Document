# Linux下的Vim配置


## linux下的优先级

```
   system vimrc file: "$VIM/vimrc"
     user vimrc file: "$HOME/.vimrc"
 2nd user vimrc file: "~/.vim/vimrc"
      user exrc file: "$HOME/.exrc"
       defaults file: "$VIMRUNTIME/defaults.vim"
  fall-back for $VIM: "/usr/share/vim"
```

## 在Linux下编译支持python的vim

### 获取vim源码
```
git clone https://github.com/vim/vim.git 
```
### 升级源码到最新的版本
```
cd vim
git pull && git fetch
```
### 编译安装vim

```
cd src
make distclean
// 设置编译配置信息
./configure --with-features=huge \
--enable-python3interp \
--enable-pythoninterp \ --with-python-config-dir=/usr/lib/python2.7/config-x86_64-linux-gnu/ \
--enable-rubyinterp \
--enable-luainterp \
--enable-perlinterp \
--with-python3-config-dir=/usr/lib/python3.5/config-3.5m-x86_64-linux-gnu/ \
--enable-multibyte \
--enable-cscope \
--prefix=/usr/local/vim/

make

make install
```

如果以上编译有问题，再执行以下命令：
```
sudo apt-get install python-dev

sudo apt-get install python3-dev
```

编译的参数说明如下：
>--with-features=huge：支持最大特性\
--enable-rubyinterp：打开对ruby编写的插件的支持
--enable-pythoninterp：打开对python编写的插件的支持\
--enable-python3interp：打开对python3编写的插件的支持\
--enable-luainterp：打开对lua编写的插件的支持\
--enable-perlinterp：打开对perl编写的插件的支持\
--enable-multibyte：打开多字节支持，可以在Vim中输入中文\
--enable-cscope：打开对cscope的支持\
--with-python-config-dir=/usr/lib/python2.7/config-x86_64-linux-gnu/ 指定python 路径\
--with-python-config-dir=/usr/lib/python3.5/config-3.5m-x86_64-linux-gnu/ 指定python3路径\
--prefix=/usr/local/vim：指定将要安装到的路径(自行创建)

## 安装vim插件YouCompleteMe
通过vim插件管理下载到了YouCompleteMe的源码，现在需要编译，所需环境：
cmake clang pythondev

### 编译clang

安装必须的编程环境：
```
sudo apt install build-essential subversion cmake python3-dev libncurses5-dev libxml2-dev libedit-dev swig doxygen graphviz xz-utils
```

从官方获取稳定版本的源码：
```
cd ~
mkdir llvm_all && cd llvm_all
svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_700/final llvm
 
cd llvm/tools
svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_700/final clang

//后面的不是必须的，可以不下载编译
cd llvm/projects
svn co http://llvm.org/svn/llvm-project/compiler-rt/tags/RELEASE_700/final compiler-rt
svn co http://llvm.org/svn/llvm-project/libcxx/tags/RELEASE_700/final libcxx
svn co http://llvm.org/svn/llvm-project/libcxxabi/tags/RELEASE_700/final libcxxabi
svn co http://llvm.org/svn/llvm-project/polly/tags/RELEASE_700/final polly
svn co http://llvm.org/svn/llvm-project/lld/tags/RELEASE_700/final lld
svn co http://llvm.org/svn/llvm-project/openmp/tags/RELEASE_700/final openmp
svn co http://llvm.org/svn/llvm-project/libunwind/tags/RELEASE_700/final libunwind
```

编译源码（时间会有些长）
```
cd ~/llvm_all
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_BUILD_DOCS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local/clang_7.0.0 ../llvm

make -j 8
sudo make install/strip
```

将clang加入环境变量
```
cd ~
echo 'export PATH=/usr/local/clang_7.0.0/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/clang_7.0.0/lib:LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### 编译YouCompleteMe
- 可以使用python
```
python install.py --clang-completer
```
- 使用install.sh
```
./install.sh --clang-completer --system-libclang
```

以上二选一
