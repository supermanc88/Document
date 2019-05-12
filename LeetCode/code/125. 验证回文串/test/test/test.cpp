#include <stdio.h>
#include <cstring>

bool isPalindrome(char * s){

	if(s == NULL)
	{
		return true;
	}

	int sLen = strlen(s);

	for(int i=0, j=sLen-1; i<j; i++,j--)
	{
		// 如果不是数字或字母则跳过
		if(!(s[i]>='0' && s[i]<='9' || s[i] >= 'A' && s[i] <= 'Z' || s[i] >= 'a' && s[i] <= 'z'))
		{
			// i动 j不动，故j++一次
			j++;
			continue;
		}

		if(!(s[j]>='0' && s[j]<='9' || s[j] >= 'A' && s[j] <= 'Z' || s[j] >= 'a' && s[j] <= 'z'))
		{
			// j动 i不动，故i--一次
			i--;
			continue;
		}

		// 全部转大写
		if(s[i] >= 'a' && s[i] <= 'z')
		{
			s[i] -= 32;
		}

		if(s[j] >= 'a' && s[j] <= 'z')
		{
			s[j] -= 32;
		}

		if(s[i] != s[j])
		{
			return false;
		}

	}

	return true;

}


int main(int argc, char* argv[])
{
	isPalindrome("0P");
	return 0;
}
