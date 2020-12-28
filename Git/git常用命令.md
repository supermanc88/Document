# Git常用命令



## 保存当前代码，但不提交

情景：在一个版本上修改bug，结果修改了很多之后，发现整个软件不能运行了，现在想暂存下这些代码，恢复到修改之前的代码



```shell
git stash save "修改信息"
```



![image-20201210193321666](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20201210193321666.png)

这样之后，代码就回到上一个commit了。


## 解决 git pull/push 每次都要输入用户名密码的问题

```shell
git config --global credential.helper store
```

## 自动添加修改的文件并提交

把暂存区的所有被修改或者已删除的且已经被git管理的文档提交提交到分支，须输入描述信息（可省略 git add 过程）

```shell
git commit -a -m "comment"
```





## 撤销git add

add某个文件后，又反悔不想添加的时候

```shell
git reset --mixed				// 这样文件退出暂存区，但是修改保留(这个应该是最好用的方式)

git reset HEAD <file>			// file文件取消暂存
```



## git打补丁



### patch和diff的区别

Git提供了两种补丁方案，一是用git diff生成的UNIX标准补丁`.diff`文件，二是`git format-patch`生成的Git专用`.patch`文件。



`.diff`文件只是记录文件改变的内容，不带有commit记录信息，多个commit可以合并成一个diff文件。

`.patch`文件带有记录文件改变的内容，也带有commit记录信息，每个commit对应一个patch文件。



### 创建patch和diff

#### 1、创建patch 文件的常用命令行

##### *某次提交（含）之前的几次提交：

```shell
git format-patch 【commit sha1 id】-n
```

n指从sha1 id对应的commit开始算起n个提交。 eg：

```shell
git format-patch  2a2fb4539925bfa4a141fe492d9828d030f7c8a8 -2
```

##### *某个提交的patch：

```shell
git format-patch 【commit sha1 id】 -1
```

eg：

```shell
git format-patch  2a2fb4539925bfa4a141fe492d9828d030f7c8a8 -1
```

##### *某两次提交之间的所有patch:

```shell
git format-patch 【commit sha1 id】..【commit sha1 id】 
```

eg：

```shell
git format-patch  2a2fb4539925bfa4a141fe492d9828d030f7c8a8..89aebfcc73bdac8054be1a242598610d8ed5f3c8
```

#### 2、创建diff文件的常用方法

##### 使用命令行

```shell
git diff  【commit sha1 id】 【commit sha1 id】 >  【diff文件名】
```

eg：

```shell
git diff  2a2fb4539925bfa4a141fe492d9828d030f7c8a8  89aebfcc73bdac8054be1a242598610d8ed5f3c8 > patch.diff
```

### 应用patch 和 diff

#### 相关命令行

##### 检查patch/diff是否能正常打入:

```shell
git apply --check 【path/to/xxx.patch】
git apply --check 【path/to/xxx.diff】
```

##### 打入patch/diff:

```shell
git apply 【path/to/xxx.patch】
git apply 【path/to/xxx.diff】
```

或者

```shell
git  am 【path/to/xxx.patch】
```

### 冲突解决

在打补丁过程中有时候会出现冲突的情况，有冲突时会打入失败，如图：

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/164d0feb20bb887e)



此时需要解决冲突： 

1、首先使用 以下命令行，自动合入 patch 中不冲突的代码改动，同时保留冲突的部分：

```
git  apply --reject  xxxx.patch
```

可以在终端中显示出冲突的大致代码：

![img](https://raw.githubusercontent.com/supermanc88/ImageSources/master/164d0feb478f6a3a)

同时会生成后缀为 .rej 的文件，保存没有合并进去的部分的内容，可以参考这个进行冲突解决。 

2、解决完冲突后删除后缀为 .rej  的文件，并执行`git add.`添加改动到暂存区. 

3、接着执行`git am --resolved`或者`git am --continue`



说明：在打入patch冲突时，可以执行`git am --skip`跳过此次冲突，也可以执行`git am --abort`回退打入patch的动作，还原到操作前的状态。


