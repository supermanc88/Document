#include <iostream>

/*
输入: [0,1,0,3,12]
输出: [1,3,12,0,0]
 */

/*
 *题解：
 *首先是每轮把一个0放到最后，故循环numssize次
 *其次，在内部循环中找到0开始的位置，它和它右边的数据换位即可
 */

void moveZeroes(int* nums, int numsSize) 
{
	int i = 0;
	int j = 0;

	for(i = 0; i < numsSize; i++)
	{
		for(j = 0; j < numsSize-1; j++)
		{
			if(nums[j] != 0)
			{
				continue;
			}
			int temp = nums[j];
			nums[j] = nums[j + 1];
			nums[j + 1] = temp;
		}
	}
}


int main()
{
	int nums[5] = { 0, 1, 0, 3, 12 };

	moveZeroes(nums, 5);

	for(int i=0; i < 5; i++)
	{
		printf("%d ", nums[i]);
	}

    std::cout << "Hello World!\n"; 
}