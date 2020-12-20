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