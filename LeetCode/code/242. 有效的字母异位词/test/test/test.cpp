#include <stdio.h>
#include <cstring>

bool isAnagram(char * s, char * t)
{
	int c[26] = {0};

	int sLen = strlen(s);
	int tLen = strlen(t);
    
    if(sLen != tLen)
    {
        return false;
    }

	for(int i=0; i<sLen; i++)
	{
		c[s[i] - 'a'] += 1;
		c[t[i] - 'a'] -= 1;
	}

	for(int i=0; i<26; i++)
	{
		if(c[i] != 0)
		{
			return false;
		}
	}

	return true;

}


int main(int argc, char* argv[])
{
	return 0;
}
