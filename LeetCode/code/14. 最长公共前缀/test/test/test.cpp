#include <stdio.h>
#include <cstring>
#include <stdlib.h>

char * longestCommonPrefix(char ** strs, int strsSize)
{
	/*
	 * 思路：
	 * 先比较第一和第二个，如果有公共的前缀，则用前缀和第三个比较
	 * 如果没有公共前缀，就直接返回，后面的不需要比较
	 */

	char * temp = (char*)malloc(128);			//用来存储公共前缀
	memset(temp, 0, 128);

	if (strsSize == 0)
	{
		return "";
	}

	strcpy(temp, strs[0]);
	int count = strlen(temp);		//用来计数，公共前缀字符数为0时，则没有

	for (int i = 0; i < strsSize; i++)
	{
		char * currentStr = strs[i];

		if (count == 0)
		{
			return "";
		}
		int j;
		for (j = 0; j < count; j++)
		{
			if (temp[j] != currentStr[j])
			{
				break;
			}
		}

		count = j;
		temp[j] = 0;
	}

	return temp;

}


int main(int argc, char* argv[])
{
	char * strs[3] = { "flower", "", "flight" };

	longestCommonPrefix(strs, 3);

	return 0;
}
