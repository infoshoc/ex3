#include <stdlib.h>
#include <assert.h>
#include "my_set.h"

typedef struct MySetNode_t {
	MySetElement element;
	struct MySetNode_t *next;
} *MySetNode, MySetNode_t;

typedef struct MySet_t {
	copyMySetElements copyElement;
	freeMySetElements freeElement;
	compareMySetElements compareElements;
	MySetNode head, iterator;
} MySet_t;

#define MY_SET_ALLOCATION(type, variable, error) \
	do { \
		if(NULL == (variable = (type*)malloc(sizeof(type)))) { \
			return error; \
		} \
	}while(false)

MySet mySetCreate(copyMySetElements copyElement, freeMySetElements freeElement, compareMySetElements compareElements) {
	if (copyElement == NULL ||
			freeElement == NULL ||
			compareElements == NULL) {
		return NULL;
	}

	MySet set;
	MY_SET_ALLOCATION(MySet_t, set, NULL);

	set->copyElement = copyElement;
	set->freeElement = freeElement;
	set->compareElements = compareElements;
	set->head = NULL;
	set->iterator = NULL;
	return set;
}

MySet mySetCopy(MySet set){
	if (set == NULL){
		return NULL;
	}
	MySet newSet = mySetCreate(set->copyElement, set->freeElement,
			set->compareElements);
	if (newSet == NULL) {
		return NULL;
	}

	for (MySetNode current = set->head; current != NULL; current = current->next){
		MySetResult adding = mySetAdd(newSet, current->element);
		if (adding == MY_SET_OUT_OF_MEMORY){
			mySetDestroy(newSet);
			return NULL;
		}
		assert(adding == MY_SET_SUCCESS);
	}

	return newSet;
}

void mySetDestroy(MySet set) {
	if(set == NULL) {
		return;
	}

	while (set->head != NULL) {
		MySetNode current = set->head;
		set->head = set->head->next;
		set->freeElement(current->element);
		free(current);
	}
	free(set);
}

int mySetGetSize(MySet set){
	if (set==NULL){
		return -1;
	}
	MySetNode position = set->head;
	int size = 0;
	while (position!=NULL){
		++size;
		position = position->next;
	}
	return size;
}

bool mySetIsIn(MySet set, MySetElement element) {
	if (set == NULL || element == NULL) {
		return false;
	}
	for (MySetNode iterator = set->head; iterator != NULL; iterator = iterator->next) {
		if (set->compareElements(iterator->element, element) == 0) {
			return true;
		}
	}

	return false;
}

MySetElement mySetGetFirst(MySet set){
	if (set==NULL){
		return NULL;
	}
	set->iterator = set->head;
	return set->iterator ? set->iterator->element : NULL;
}

MySetElement mySetGetNext(MySet set) {
	if (set == NULL ||
			set->iterator == NULL) {
		return NULL;
	}
	set->iterator = set->iterator->next;
	return set->iterator ? set->iterator->element : NULL;
}

MySetElement mySetGetCurrent(MySet set){
	if (set == NULL ||
		set->iterator == NULL) {
		return NULL;
	}
	return set->iterator->element;
}

MySetResult mySetAdd(MySet set, MySetElement element) {
	if (set == NULL || element == NULL) {
		return MY_SET_NULL_ARGUMENT;
	}
	if (mySetIsIn(set, element)) {
		return MY_SET_ITEM_ALREADY_EXISTS;
	}

	// Create new node
	MySetNode node;
	MY_SET_ALLOCATION(MySetNode_t, node, MY_SET_OUT_OF_MEMORY);
	if (NULL == (node->element = set->copyElement(element))) {
		free(node);
		return MY_SET_OUT_OF_MEMORY;
	}

	if (set->head == NULL || set->compareElements(set->head->element, element) > 0) {
		//need to push front
		node->next = set->head;
		set->head = node;
	} else {
		assert(set->head != NULL);
		assert(set->compareElements(set->head->element, element) < 0);
		// find last position where element is lower then ours or last element
		MySetNode position;
		for (position = set->head;
				position->next != NULL &&
						set->compareElements(position->next->element, element) < 0;
				position = position->next);
		assert(position != NULL);
		assert(position->next == NULL || set->compareElements(position->next->element, element) > 0);
		// insert after position
		node->next = position->next;
		position->next = node;
	}
	return MY_SET_SUCCESS;
}

MySetResult mySetRemove(MySet set, MySetElement element){
	if (set==NULL || element == NULL){
		return MY_SET_NULL_ARGUMENT;
	}
	if (!mySetIsIn(set, element)){
		return MY_SET_ITEM_DOES_NOT_EXIST;
	}
	MySetElement elementFound = mySetExtract(set, element);
	set->freeElement(elementFound);
	return MY_SET_SUCCESS;
}


MySetElement mySetExtract(MySet set, MySetElement element) {
	if (set == NULL || !mySetIsIn(set, element)) {
		return NULL;
	}

	assert(set->head != NULL);
	MySetElement result;
	MySetNode node; // node we want to deallocate
	if (set->compareElements(set->head->element, element) == 0) {
		// pop front
		result = set->head->element;
		node = set->head;
		// extracting
		set->head = set->head->next;
	} else {
		MySetNode position;
		for (position = set->head;
				position->next != NULL && set->compareElements(position->next->element, element) != 0;
				position = position->next);

		assert(position->next != NULL && set->compareElements(position->next->element, element) == 0);
		result = position->next->element;
		if (position->next->next == NULL) {
			node = position->next;
			// extracting
			position->next = NULL;
		} else {
			position = position->next;
			node = position->next;
			// extracting
			assert(position->next != NULL);
			position->element = position->next->element;
			position->next = position->next->next;
		}

	}
	free(node);
	return result;
}

MySetResult mySetClear(MySet set){
	if (set==NULL){
		return MY_SET_NULL_ARGUMENT;
	}
	MySetElement clearElement = mySetGetFirst(set);
	while (clearElement != NULL){
		MySetResult clearing = mySetRemove(set, clearElement);
		assert(clearing == MY_SET_SUCCESS);
		clearElement = mySetGetFirst(set);
	}
	return MY_SET_SUCCESS;
}

MySet mySetFilter(MySet set, logicalCondition condition) {
	if (set == NULL || condition == NULL) {
		return NULL;
	}

	MySet result = mySetCreate(set->copyElement, set->freeElement, set->compareElements);
	if (result == NULL) {
		return NULL;
	}

	for (MySetNode position = set->head; position != NULL; position = position->next) {
		if (condition(position->element)) {
			MySetResult resultAddResult = mySetAdd(result, position->element);
			if (resultAddResult != MY_SET_SUCCESS) {
				assert(resultAddResult == MY_SET_OUT_OF_MEMORY);
				mySetDestroy(result);
				return NULL;
			}
		}
	}
	return result;
}
