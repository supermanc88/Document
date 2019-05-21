#include <stdio.h>


struct ListNode {
    int val;
    struct ListNode *next;
};



bool isPalindrome(struct ListNode* head){

	if(head == NULL || head->next == NULL)
	{
		return true;
	}

	struct ListNode * slow = head;
	struct ListNode * fast = head;

	// 先定位到中点 slow开始就是下半部分

	while(fast != NULL)
	{
		slow = slow->next;
		fast = fast->next ? fast->next->next : NULL;
	}

	// 翻转slow链表 只有多于1个节点才开始翻转
	struct ListNode * pre = NULL;
	struct ListNode * cur = slow;
	struct ListNode * next = slow;
	while(next != NULL)
	{
		next = cur->next;
		cur->next = pre;

		pre = cur;
		cur = next;
	}

	while(pre != NULL)
	{
		if(pre->val != head->val)
		{
			return false;
		}

		pre = pre->next;
		head = head->next;
	}

	return true;

}


int main(int argc, char* argv[])
{
	struct ListNode A = {1,NULL};
	struct ListNode B = {2,NULL};
	struct ListNode C = {2,NULL};
	struct ListNode D = {1,NULL};
	A.next = &B;
	B.next = &C;
	C.next = &D;
	D.next = NULL;

	isPalindrome(&A);
	return 0;
}
