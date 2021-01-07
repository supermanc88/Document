# Linux执行shell命令并获取输出

关键函数：popen pclose

popen通过创建一个管道，将shell命名传给`/bin/sh`加`-c`参数执行，执行成功后，返回一个打开的文件流。



```c
#include <stdio.h>
#include <string.h>

void execute_commond(char *cmd, char *result)
{
	char ps[1024] = {0};
	char buf_result[1024] = {0};

	FILE *ptr;

	strcpy(ps, cmd);


	if ((ptr = popen(ps, "r")) != NULL) {
		while (fgets(buf_result, 1024, ptr) != NULL) {
			strcat(result, buf_result);
			if (strlen(result) > 1024)
				break;
		}

		pclose(ptr);
	} else {
		printf("popen error\n");
	}
	
}


int main()
{
	char result[1024] = {0};

	execute_commond("dmidecode -s system-uuid", result);

	printf("%s\n", result);
		
	return 0;

}

```

