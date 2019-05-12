#include <stdio.h>
#include <string.h>

int firstUniqChar(char * s)
{
	int c[26] = {0};

	for(int i=0; i < strlen(s); i++)
	{
		c[s[i] - 'a']++; 
	}

	for(int i=0; i < strlen(s); i++)
	{
		if(c[s[i] - 'a'] == 1)
		{
			return i;
		}
	}
	return -1;
}

int main(int argc, char* argv[])
{
	return 0;	
}
