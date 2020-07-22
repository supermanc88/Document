#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define FIFO_NAME "/var/tmp/fifo_test"

int main()
{
    int ret = 0;
    int pipefd = 0;
    // 判断fifo文件是否已存在
    if (access(FIFO_NAME, F_OK) == -1) {
        // 不存在则创建
        ret = mkfifo(FIFO_NAME, 0666);

        if (ret == -1) {
            perror("mkfifo error");
            return -1;
        }
    }


    // 以写端打开
    pipefd = open(FIFO_NAME, O_WRONLY);

    printf("write open success\n");

    if (pipefd == -1) {
        perror("open error");
        return -1;
    }

    char buf[BUFSIZ] = {0};

    while (1) {
        // 从控制台获取输入
        int read_bytes = read(STDIN_FILENO, buf, BUFSIZ);

        if( read_bytes == -1 ) {
            perror("read error");
            close(pipefd);
            return -1;
        }

        // 写到管道
        int write_bytes = write(pipefd, buf, read_bytes);
    }

    close(pipefd);



    return 0;
}
