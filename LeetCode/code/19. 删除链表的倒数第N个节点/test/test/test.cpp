#include <stdio.h>
#include <cstring>
#include <cstdlib>

//Definition for singly-linked list.

struct ListNode {
    int val;
	struct ListNode *next;
};

// 双指针解法
struct ListNode* removeNthFromEnd(struct ListNode* head, int n)
{
	// struct ListNode * A = new struct ListNode;
	struct ListNode * A = (struct ListNode *)malloc(sizeof(struct ListNode));
	A->next = head;
	struct ListNode * first, *second;
	first = A;
	second = A;

	for (int i = 1; i <= n + 1; i++)
	{
		first = first->next;
	}

	while (first)
	{
		first = first->next;
		second = second->next;
	}

	second->next = second->next->next;

	return A->next;

}


struct ListNode* removeNthFromEnd2(struct ListNode* head, int n)
{
	int count = 0;

	struct ListNode * temp = head;

	while (temp !=0)
	{
		count++;
		temp = temp->next;
	}

	if (count == n)
	{
		return head->next;
	}

	int i = 1;
	for (temp = head; temp->next != NULL;)
	{
		if (i == count - n)	// 找到要删除的前一个
		{
			temp->next = temp->next->next;
			break;
		}

		temp = temp->next;
		i++;

	}

	return head;
}

// 下面这个是头节点不算数据的

struct ListNode* removeNthFromEnd1(struct ListNode* head, int n){
	/*
	 * 思路：遍历链表中有多少个节点，然后找到倒数第n个
	 */
	int count = 0;
	struct ListNode * temp = head;
	while (temp->next)
	{
		count++;
		temp = temp->next;
	}
	int i = 0;
	for (temp = head; temp->next != NULL;)
	{
		if (i == count-n)	// 找到要删除的前一个
		{
			temp->next = temp->next->next;
			break;
		}

		temp = temp->next;
		i++;

	}
	return head;
}

int main(int argc, char* argv[])
{
	struct ListNode head, A, B, C;
	memset(&head, 0, sizeof(ListNode));
	memset(&A, 0, sizeof(ListNode));
	memset(&B, 0, sizeof(ListNode));
	memset(&C, 0, sizeof(ListNode));

	A.val = 1;
	B.val = 2;
	C.val = 3;

	head.next = &A;
	A.next = &B;
	B.next = &C;


	removeNthFromEnd(&head, 3);

	return 0;
}
