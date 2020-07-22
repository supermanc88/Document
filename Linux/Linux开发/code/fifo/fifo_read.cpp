#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define FIFO_NAME "/var/tmp/fifo_test"


int main()
{
    int pipefd = 0;

    pipefd = open(FIFO_NAME, O_RDONLY);

    printf("read open success\n");

    char buf[BUFSIZ] = {0};

    while (1) {
        int read_bytes = read(pipefd, buf, BUFSIZ);

        printf("read bytes = %d\n", read_bytes);

        if (read_bytes == 0) {
            close(pipefd);
            printf("write peer close fd\n");
            return 0;
        }

        write(STDOUT_FILENO, buf, read_bytes);
    }

    close(pipefd);

    return 0;
}