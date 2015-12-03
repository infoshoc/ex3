/*
 * my_list.c
 *
 *  Created on: Dec 3, 2015
 *      Author: Infoshoc_2
 */

#include <stdlib.h>

struct MyList_t {
	MyListElement value;
	MyList next;
};

int MyListGetSize(const MyList myList) {
	int length = 0;
	for (MyList iterator = myList; iterator != NULL; ++iterator) {
		++length;
	}
	return length;
}
