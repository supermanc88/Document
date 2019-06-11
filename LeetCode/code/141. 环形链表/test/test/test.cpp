#include <stdio.h>


struct ListNode {
    int val;
    struct ListNode *next;
};

bool hasCycle(struct ListNode *head) {

	if (head == NULL || head->next == NULL)
	{
		return false;
	}

	/*
	 * 思路：快慢指针，慢指针每次走一步
	 * 快指针每次走两步
	 * 如果有环的话，快指针一定会追到慢指针
	 */
	struct ListNode * slow = head;
	struct ListNode * fast = head;

	while (fast != NULL)
	{
		slow = slow->next;
		fast = fast->next ? fast->next->next : NULL;

		if (fast == slow)
		{
			return true;
		}
	}

	return false;

}

int main(int argc, char* argv[])
{
	return 0;
}
