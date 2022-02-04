#ifndef LISTS_H
#define LISTS_H

typedef struct {
	char *head;
	List *tail;
} List;

void append(List *list, char* element);
void access(List *list, int index);
	

#endif