#ifndef MY_SET_H_
#define MY_SET_H_

#include <stdbool.h>

/**
* Generic mySet Container
*
* Implements a mySet container type.
* The mySet has an internal iterator for external use. For all functions
* where the state of the iterator after calling that function is not stated,
* it is undefined. That is you cannot assume anything about it.
*
* The following functions are available:
*   mySetCreate		- Creates a new empty mySet
*   mySetCopy		- Copies an existing mySet
*   mySetDestroy		- Deletes an existing mySet and frees all resources
*   mySetGetSize		- Returns the size of a given mySet
*   mySetIsIn		- returns weather or not an item exists inside the mySet.
*   				  This resets the internal iterator.
*   mySetGetFirst	- Sets the internal iterator to the first element in the
*   				  mySet, and returns it.
*   mySetGetNext		- Advances the internal iterator to the next element and
*   				  returns it.
*   mySetAdd			- Adds a new element to the mySet.
*   mySetRemove		- Removes an element which matches a given element (by the
*   				  compare function). Resets the internal iterator.
*	 mySetClear		- Clears the contents of the mySet. Frees all the elements of
*	 				  the mySet using the free function.
* 	 SET_FOREACH	- A macro for iterating over the mySet's elements.
*/

/** Type for defining the mySet */
typedef struct MySet_t *MySet;

/** Type used for returning error codes from mySet functions */
typedef enum MySetResult_t {
	MY_SET_SUCCESS,
	MY_SET_OUT_OF_MEMORY,
	MY_SET_NULL_ARGUMENT,
	MY_SET_ITEM_ALREADY_EXISTS,
	MY_SET_ITEM_DOES_NOT_EXIST
} MySetResult;

/** Element data type for mySet container */
typedef void* MySetElement;

/** Type of function for copying an element of the mySet */
typedef MySetElement(*copyMySetElements)(MySetElement);

/** Type of function for deallocating an element of the mySet */
typedef void(*freeMySetElements)(MySetElement);

/**
* Type of function used by the mySet to identify equal elements.
* This function will be used to deciding the iteration order of the mySet.
* This function should return:
* 		A positive integer if the first element is greater;
* 		0 if they're equal;
*		A negative integer if the second element is greater.
*/
typedef int(*compareMySetElements)(MySetElement, MySetElement);

/**
* Type of function to pass as an argument to the filtering function.
* This function should return true if the element passes the filter, false
* otherwise.
*/
typedef bool(*logicalCondition) (MySetElement);

/**
* mySetCreate: Allocates a new empty mySet.
*
* @param copyElement - Function pointer to be used for copying elements into
*  	the mySet or when copying the mySet.
* @param freeElement - Function pointer to be used for removing elements from
* 		the mySet
* @param compareElements - Function pointer to be used for comparing elements
* 		inside the mySet. Used to check if new elements already exist in the mySet.
* @return
* 	NULL - if one of the parameters is NULL or allocations failed.
* 	A new Set in case of success.
*/
MySet mySetCreate(copyMySetElements copyElement, freeMySetElements freeElement, compareMySetElements compareElements);

/**
* mySetCopy: Creates a copy of target mySet.
*
* @param set - Target mySet.
* @return
* 	NULL if a NULL was sent or a memory allocation failed.
* 	A Set containing the same elements as set otherwise.
*/
MySet mySetCopy(MySet set);

/**
* mySetDestroy: Deallocates an existing mySet. Clears all elements by using the
* stored free function.
*
* @param set - Target mySet to be deallocated. If set is NULL nothing will be
* 		done
*/
void mySetDestroy(MySet set);

/**
* mySetGetSize: Returns the number of elements in a mySet
* @param set - The mySet which size is requested
* @return
* 	-1 if a NULL pointer was sent.
* 	Otherwise the number of elements in the mySet.
*/
int mySetGetSize(MySet set);

/**
* mySetIsIn: Checks if an element exists in the mySet. The element will be
* considered in the mySet if one of the elements in the mySet it determined equal
* using the comparison function used to initialize the mySet.
*
* @param set - The mySet to search in
* @param element - The element to look for. Will be compared using the
* 		comparison function.
* @return
* 	false - if the input set is null, or if the element was not found.
* 	true - if the element was found in the mySet.
*/
bool mySetIsIn(MySet set, MySetElement element);

/**
*	mySetGetFirst: Sets the internal iterator (also called current element) to
*	the first element in the mySet.
*	Use this to start iteraing over the mySet.
*	Use (To continue iteration use mySetGetNext)
*
* @param set - The mySet for which to set the iterator and return the first
* 		element. By "first" element we mean the element with the smallest value
*       among all elements following the comparison funcrion.
* @return
* 	NULL if a NULL pointer was sent or the mySet is empty.
* 	The first element of the mySet otherwise
*/
MySetElement mySetGetFirst(MySet set);

/**
*	mySetGetNext: Advances the mySet iterator to the next element and returns it.
*                 Again, iteration is done by order defined by the comparison function.
* @param set - The mySet for which to advance the iterator
* @return
* 	NULL if reached the end of the mySet, or the iterator is at an invalid state
* 	or a NULL sent as argument
* 	The next element on the mySet in case of success
*/
MySetElement mySetGetNext(MySet set);

/**
* mySetGetCurrent: Returns the element pointed by the iterator.
* @param set - The mySet from which to get the current element
* @return
*   NULL if the iterator is at an invalid state
*   or a NULL sent as argument
*   The current element in the mySet in case of success
*/
MySetElement mySetGetCurrent(MySet set);

/**
*	mySetAdd: Adds a new element to the mySet.
*  Iterator's value is undefined after this operation.
*
* @param set - The mySet for which to add an element
* @param element - The element to insert. A copy of the element will be
* 		inserted as supplied by the copying function which is given at
* 		initialization.
* @return
* 	MY_SET_NULL_ARGUMENT if a NULL was sent as set
* 	MY_SET_OUT_OF_MEMORY if an allocation failed (Meaning the function for copying
* 	an element failed)
*  MY_SET_ITEM_ALREADY_EXISTS if an equal item already exists in the mySet
* 	MY_SET_SUCCESS the element has been inserted successfully
*/
MySetResult mySetAdd(MySet set, MySetElement element);

/**
* 	mySetRemove: Removes an element from the mySet. The element is found using the
* 	comparison function given at initialization. Once found, the element is
* 	removed and deallocated using the free function supplied at initialzation.
*   Iterator's value is undefined after this operation.
*
* @param set -
* 	The mySet to remove the element from.
* @param element
* 	The element to remove from the mySet. The element will be freed using the
* 	free function given at initialization.
* @return
* 	MY_SET_NULL_ARGUMENT if a NULL was sent as set
* 	MY_SET_ITEM_DOES_NOT_EXIST if the element doesn't exist in the mySet
* 	MY_SET_SUCCESS if the element was successfully removed.
*/
MySetResult mySetRemove(MySet set, MySetElement element);

/**
*   mySetExtract: Removes an element from the mySet. The element is found using the
*   comparison function given at initialization. Once found, the element is
*   removed from the mySet **but not deallocated**.
*   Iterator's value is undefined after this operation.
*
* @param set -
*   The mySet to remove the element from.
* @param element
*   The element to remove from the mySet.
* @return
*   NULL if a NULL was sent as set or if the element doesn't exist in the mySet,
*   the removed element otherwise.
*/
MySetElement mySetExtract(MySet set, MySetElement element);

/**
* mySetClear: Removes all elements from target mySet.
* The elements are deallocated using the stored free function
* @param set
* 	Target mySet to remove all element from
* @return
* 	MY_SET_NULL_ARGUMENT - if a NULL pointer was sent.
* 	MY_SET_SUCCESS - Otherwise.
*/
MySetResult mySetClear(MySet set);

/**
* mySetFilter: Creates a new mySet which contains the elements in the
* source mySet that satisfy the logical condition passed as an argument.
* @param set
*   Source mySet to filter.
* @return
*   NULL if a NULL pointer was sent or memory allocation failed,
*   the new filtered set otherwise.
*/
MySet mySetFilter(MySet set, logicalCondition condition);

/*!
* Macro for iterating over a mySet.
* Declares a new iterator for the loop.
*/
#define MY_SET_FOREACH(type,iterator,set) \
	for(type iterator = mySetGetFirst(set) ; \
		iterator ;\
		iterator = mySetGetNext(set))

#endif /* MY_SET_H_ */
