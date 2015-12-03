#include <stdlib.h>
#include "my_list.h"
#include "my_set.h"

struct MySet_t {
	MyList elements;
	compareMySetElements compareElements;
};

MySet mySetCreate(copyMySetElements copyElement, freeMySetElements freeElement, compareMySetElements compareElements) {
	MySet mySet = (MySet)malloc(sizeof(*mySet));
	if (mySet == NULL) {
		return NULL;
	}

	mySet->compareElements = compareElements;
	mySet->elements = MyListCreate(copyElement, freeElement);
	if (mySet->elements == NULL) {
		free(mySet);
		return NULL;
	}
	return mySet;
}

void mySetDestroy(MySet set) {
	if(set == NULL) {
		return;
	}

	myListDestroy(set->elements);
	free(set);
}

bool mySetIsIn(MySet set, MySetElement element) {
	return myListIsIn(set->elements, (MyListElement)element, set->compareElements);
}

MySetElement mySetGetNext(MySet set) {
	return myListGetNext(set->elements);
}
