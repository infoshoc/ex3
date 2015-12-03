/*
 * my_list.h
 *
 *  Created on: Dec 3, 2015
 *      Author: Infoshoc_2
 */

#ifndef MY_LIST_H_
#define MY_LIST_H_

/** Type for defining the list */
typedef struct MyList_t *MyList;

/** Type used for returning error codes from list functions */
typedef enum MyListResult_t {
	LIST_SUCCESS,
	LIST_OUT_OF_MEMORY
} MyListResult;

/** Element data type for list container */
typedef void* MyListElement;

/** Type of function for copying an element of the list */
typedef MyListElement(*CopyMyListElement)(MyListElement);

/** Type of function for deallocating an element of the list */
typedef void(*FreeMyListElement)(MyListElement);

/**
 * Allocates a new MyList
 *
 * @param copyElement Function pointer to be used for copying elements into
 * the list or when copying the list.
 * @param freeElement Function pointer to be used for removing elements from
 * the list.
 * @return
 * 	NULL - if one of the parameters is NULL or allocations failed.
 * 	A new MyList in case of success.
 */
MyList MyListCreate(CopyMyListElement copyElement, FreeMyListElement freeElement);

/**
 * Returns the number of elements in a list
 *
 * @param myList  The target list which size if requested
 * @return
 * 	-1 - if a NULL pointer was sent
 * 	OTherwise the number of elements in the list
 */
int MyListGetSize(MyList myList);

/**
 * Sets the internal iterator to the first element and retrieves it.
 *
 * @param myList The list for which to set the iterator and return the first
 * @return
 * 	NULL if a NULL pointer was sent or the list is empty
 * 	The first element of the list otherwise
 */
MyListElement MyListGetFirst(MyList myList);

/**
 * Advances the list's iterator to the next element and return it
 *
 * @param myList
 * @return
 * NULL if reached the end of the list, the iterator is at invalid state or
 * a NULL sent as an argument
 * Otherwise the next element on the list
 */
MyListElement myListGetNext(MyList myList);

/**
 * Adds a new element to the list, the new element will be place right before
 * the current element
 * @param myList
 * @param element
 * @return
 * LIST_NULL_ARGUMENT if a NULL was sent as
 */
MyListResult myListInsertBeforeCurrent(MyList myList, MyListElement element);

MyListResult myListClear(MyList mylist);

void myListDestroy(MyList Mylist);

/**
 * Macro for iterating over a list.
 */
#define MY_LIST_FOREACH(type,iterator,list) \
	for(type iterator = listGetFirst(list) ; \
		iterator ;\
		iterator = listGetNext(list))

#endif /* MY_LIST_H_ */
