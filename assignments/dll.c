#include <stdio.h>

struct Node {
	int data;
	struct Node* next;
	struct Node* previous;
};
struct Node create_list (int data);
void push(struct Node** head, int data_to_push);
int pop(struct Node ** head);

int main(void) {
	// Pushing the first element of the DLL.
	// Head is set to be NULL for first push.
	struct Node* head = NULL;
	push(&head, 20);
	push(&head, 10);
	int thing = pop(&head);
	printf("%d\n", thing);
}

void push(struct Node ** head, int data_to_push) {
	struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
	(*new_node).data = data_to_push;
	(*new_node).next = *head;
	(*new_node).previous = NULL;


	//If a node existed before the push then set it's previous to the head.
	if (*head != NULL) {
		(**head).previous = new_node;
	}; 

	// Make new_node the head
	(*head) = new_node;
}

int pop(struct Node ** head) {
	// Shift the head to be the node to the right
	struct Node* new_head = (*head)->next;
	int value = (*head)->data;
	(*head) = new_head;
	// Set the head nodes previous to be null
	(**head).previous = NULL;
	return value;
}