注意： 头文件 unistd 必须在首行

在调用open函数时，在使用`O_CREAT`时，要往里面写内容时，仅使用`O_RDONLY`参数不能写入，在替换`O_RDWR`后，可写入。
