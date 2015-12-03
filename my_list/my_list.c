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
	CopyMyListElement copyElement;
	FreeMyListElement freeElement;
	MyListNode begin, iterator;
};

int MyListGetSize(const MyList myList) {
	if (myList == NULL) {
		return -1;
	}
	int length = 0;
	for (MyListNode iterator = myList->begin;
			iterator != NULL;
			iterator = iterator->next) {
		++length;
	}
	return length;
}

MyListElement myListGetNext(MyList myList) {
	if (myList == NULL || myList->iterator == NULL) {
		return NULL;
	}
	myList->iterator = myList->iterator->next;
	if (myList->iterator == NULL) {
		return NULL;
	}
	return myList->iterator->value;
}

MyListResult myListRemoveCurrent(MyList myList) {
	if (myList == NULL) {
		return MY_LIST_NULL_ARGUMENT;
	}
	if (myList->iterator == NULL) {
		return MY_LIST_INVALID_CURRENT;
	}

	//deallocate current value
	myList->freeElement(myList->iterator->value);
	MyListNode nextNode = myList->iterator->next;
	if (nextNode == NULL) {
		//deallocate current Node
		free(myList->iterator);
	} else {
		//copy next node inside current
		myList->iterator->next = nextNode->next;
		myList->iterator->value = nextNode->value;
		//deallocate next node
		free(nextNode);
	}
	myList->iterator = NULL;
	return MY_LIST_SUCCESS;
}
