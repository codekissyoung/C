#include "common.h"
struct node* init(int num){
	struct node *p;
	p = (struct node *)malloc(sizeof(struct node));
	p -> data = 10;
	p -> next = NULL;
	return p;
}



