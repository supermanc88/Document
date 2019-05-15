#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

string countAndSay(int n)
{
	if (n==1)
	{
		return "1";
	}

	//根据上一个结果求本次结果

	string preResult = countAndSay(n - 1);

	string result;
	int count = 1;

	for (int i = 0; i < preResult.size(); i++)
	{
		// 这里判断有多少个重复的字符
		if (preResult[i] == preResult[i+1])
		{
			count++;
			continue;
		}
		else
		{
			// 当找到一个不相等的时,开始拼接字符串
			result += to_string(count) + preResult[i];
			count = 1;
		}

	}

	return result;

}

int main(int argc, char* argv[])
{
	return 0;
}
