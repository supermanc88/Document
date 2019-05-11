#include <stdio.h>


/**
	输入：["h","e","l","l","o"]
	输出：["o","l","l","e","h"]
 */

/**
 *思路：第一个和最后一个换
 *第二个和倒数第二个换
 *以此类推
 */

void reverseString(char* s, int sSize)
{
	int last = sSize - 1;
	for(int i=0; i<sSize/2; i++)
	{
		char temp = s[i];
		s[i] = s[last - i];
		s[last-i] = temp;
	}
}


int main()
{
	char test[5] = {'h','e','l','l','o'};
	reverseString(test, 5);

	for(int i=0; i<5; i++)
	{
		printf("%c ", test[i]);
	}
	return 0;
}