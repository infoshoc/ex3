/*
 * my_list.c
 *
 *  Created on: Dec 3, 2015
 *      Author: Infoshoc_2
 */

#include <stdlib.h>
#include "my_list.h"

typedef struct MyListNode_t *MyListNode;
struct MyListNode_t{
	MyListElement value;
	MyListNode next;
};

struct MyList_t {
	MyListNode begin, iterator;
};

int MyListGetSize(const MyList myList) {
	int length = 0;
	for (MyList iterator = myList; iterator != NULL; ++iterator) {
		++length;
	}
	return length;
}
