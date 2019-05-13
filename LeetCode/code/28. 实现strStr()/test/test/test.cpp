#include <stdio.h>
#include <cstring>

int strStr(char * haystack, char * needle){

	if (haystack == NULL || needle == NULL || strlen(needle)==0)
	{
		return 0;
	}

	int hLen = strlen(haystack);
	int nLen = strlen(needle);

	// 如果needle长度大于haystack，肯定找不到
	if (nLen > hLen)
	{
		return -1;
	}

	int index = -1;

	for (int i = 0; i < hLen; i++)
	{
		if (needle[0] != haystack[i])
		{
			continue;
		}

		// 找到h中第一个和n第一个字符相等的位置

		//如果h剩余的长度小于n的长度，则找不到
		if (hLen - i < nLen)
		{
			return -1;
		}

		index = i;

		//判断剩余的字符串开始比较，出错则继续上面的步骤
		for (int j = 0; j < nLen; j++,index++)
		{
			if (haystack[index] != needle[j])
			{
				index = -1;
				break;
			}
		}

		if (index == -1)
		{
			continue;
		}
		else
		{
			//这里是已经找到，后面不需要再找，直接退出
			return index - nLen;
		}

	}

	return -1;
}

int main(int argc, char* argv[])
{
	strStr("hello", "ll");
	return 0;
}
