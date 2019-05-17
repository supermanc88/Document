#include <stdio.h>


struct ListNode {
    int val;
    struct ListNode *next;
};


struct ListNode* reverseList1(struct ListNode* head)
{
	if (head == NULL || head->next == NULL)
	{
		return head;
	}

	struct ListNode * pre = NULL;
	struct ListNode * cur = head;
	struct ListNode * next = cur->next;

	while (next != NULL)
	{
		next = cur->next;
		cur->next = pre;
		pre = cur;
		cur = next;
	}

	return pre;
}


struct ListNode* reverseList(struct ListNode* head){

	if (head == NULL || head->next == NULL)
	{
		return head;
	}

	struct ListNode * preNode = NULL;
	struct ListNode * q = head;
	struct ListNode * nextHead = q->next;


	while (nextHead != NULL)
	{
		nextHead = q->next;
		q->next = preNode;
		preNode = q;
		q = nextHead;
	}

	return preNode;

}


int main(int argc, char* argv[])
{
	return 0;
}
