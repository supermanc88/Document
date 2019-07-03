## 本地分支重命名

```
git branch -m oldname newname
```

## 远程分支重命名

> 如果修改远程分支，只需要将本地分支重命名为新分支名称，然后删除远程分支，再把本地分支上传即可


## 本地分支删除

```
git branch -D branchName
```

## 远程分支删除
```
git push origin :branchName (origin后面有空格)
```