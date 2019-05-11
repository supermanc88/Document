#include <stdio.h>

/*
 [
  ["5","3",".",".","7",".",".",".","."],
  ["6",".",".","1","9","5",".",".","."],
  [".","9","8",".",".",".",".","6","."],
  ["8",".",".",".","6",".",".",".","3"],
  ["4",".",".","8",".","3",".",".","1"],
  ["7",".",".",".","2",".",".",".","6"],
  [".","6",".",".",".",".","2","8","."],
  [".",".",".","4","1","9",".",".","5"],
  [".",".",".",".","8",".",".","7","9"]
]
 */

bool isValidSudoku(char** board, int boardSize, int* boardColSize)
{
	// 先遍历行
	for(int i=0; i<boardSize; i++)
	{
		for(int j=0; j<*boardColSize; j++)
		{
			for(int k=j+1; k<*boardColSize; k++)
			{
				if(board[i][j] != '.' && board[i][j] == board[i][k])
				{
					return false;
				}
			}
		}
	}

	// 再遍历列
	for(int i=0; i<boardSize; i++)
	{
		for(int j=0; j<*boardColSize; j++)
		{
			for(int k=j+1; k<*boardColSize; k++)
			{
				if(board[j][i] != '.' && board[j][i] == board[k][i])
				{
					return false;
				}
			}
		}
	}

	// 最后遍历3*3方格

	for(int i=0; i<boardSize; i+=3)
	{
		for(int j=0; j<*boardColSize; j+=3)
		{
			for (int k = 0; k < 3 * 3 - 1; k++)
			{
				for ( int l = k + 1; l < 3 * 3; l++)
				{
					if (board[k / 3 +i][k % 3 +j] == board[l / 3+i][l % 3+j] && board[k / 3 +i][k % 3 +j] != '.')
					{
						return false;
					}
				}
			}
		}
	}

	return true;

}

// 00 01 00 02 00 10 00 11 00 12
// 01 02 01 10 01 11 01 12

int Judge(int num[2][3])
{
	int i, j;
	for (i = 0; i < 2 * 3 - 1; i++)
	{
		for (j = i + 1; j < 2 * 3; j++)
		{
			if (num[i / 3][i % 3] == num[j / 3][j % 3])
			{
				return 0;
			}
		}
	}
	return 1;
}


int main()
{
	return 0;
}