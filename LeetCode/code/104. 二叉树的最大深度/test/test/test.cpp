#include <stdio.h>

 struct TreeNode {
     int val;
     struct TreeNode *left;
     struct TreeNode *right;
 };
 


int maxDepth(struct TreeNode* root){
	if(root == NULL)
	{
		return 0;
	}

	int i, j;
	i = maxDepth(root->left);
	j = maxDepth(root->right);

	return 1 + (i > j ? i : j);
}


int main(int argc, char* argv[])
{
	return 0;
}
