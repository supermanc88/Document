#include <stdio.h>
#include <cstring>

int myAtoi(char * str){
	if (str == NULL)
	{
		return 0;
	}

	// 忽略空格
	int index = 0;
	while (str[index] == ' ')
	{
		index++;
	}

	// 判断第一个数是否为数字或负号
	if (!(str[index] >= '0' && str[index] <= '9' || str[index] == '-' || str[index] == '+'))
	{
		return 0;
	}

	int strLen = strlen(str);

	// 处理负数
	if (str[index] == '-')
	{
		int i = index+1;
		int temp = 0;
		while (str[i] >= '0' && str[i] <= '9' && i < strLen)
		{
			//判断是否存在溢出
			if (temp > 2147483648 /10)
			{
				return (0-2147483648);
			}
			if (temp == 2147483648 / 10 && str[i] - 48 >= 8)
			{
				return (0-2147483648);
			}

			//str[i] - 48 字符0转数字0
			temp = temp * 10 + (str[i] - 48);

			i++;
		}
		return (0 - temp);
	}

	// 处理正数

	int i = 0;
	if (str[index] == '+')
	{
		i = index + 1;
	}
	else
	{
		i = index;
	}
	int temp = 0;
	while (str[i] >= '0' && str[i] <= '9' && i < strLen)
	{
		//判断是否存在溢出
		if (temp > 2147483647 / 10)
		{
			return 2147483647;
		}
		if (temp == 2147483647 / 10 && str[i] - 48 > 7)
		{
			return 2147483647;
		}

		//str[i] - 48 字符0转数字0
		temp = temp * 10 + (str[i] - 48);

		i++;
	}
	return temp;

}


int main(int argc, char* argv[])
{
	myAtoi("2147483646");
	return 0;
}
