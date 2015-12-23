#include "test_utilities.h"
#include <stdlib.h>
#include "my_set.h"
#include <string.h>

/** Function to be used for copying an int into the set */
static MySetElement copyString(MySetElement str) {
	if (!str) {
		return NULL;
	}
	char* copy = malloc(strlen(str) + 1);
	if (!copy) {
		return NULL;
	}
	return strcpy(copy, str);
}

/** Function to be used by the set for freeing elements */
static void freeString(MySetElement str) {
	free(str);
}

/** Function for comparing to elements in the set */
static int compareStrings(MySetElement element1, MySetElement element2) {
	return strcmp(element1, element2);
}

static bool longerThanTen(MySetElement element) {
  return strlen(element) > 10;
}

bool testMySetExample() {
  char* hello = "Hello world!";
  char* helloAgain = "Hello world!";
  char* goodbye = "Goodbye!";
  char* goodDay = "Good day!";

  MySet set = mySetCreate(copyString, freeString, compareStrings);
  ASSERT_TEST(set != NULL);
  ASSERT_TEST(mySetGetSize(set) == 0);
  ASSERT_TEST(mySetIsIn(set, hello) == false);
  ASSERT_TEST(mySetAdd(set, hello) == MY_SET_SUCCESS);
  ASSERT_TEST(mySetGetSize(set) == 1);
  ASSERT_TEST(mySetIsIn(set, helloAgain) == true);
  ASSERT_TEST(mySetAdd(set, hello) == MY_SET_ITEM_ALREADY_EXISTS);
  ASSERT_TEST(mySetGetSize(set) == 1);

  ASSERT_TEST(mySetAdd(set, goodbye) == MY_SET_SUCCESS);
  ASSERT_TEST(mySetAdd(set, goodDay) == MY_SET_SUCCESS);

  ASSERT_TEST(strcmp(goodDay, mySetGetFirst(set)) == 0);
  ASSERT_TEST(strcmp(goodbye, mySetGetNext(set)) == 0);
  ASSERT_TEST(strcmp(hello, mySetGetNext(set)) == 0);
  ASSERT_TEST(mySetGetNext(set) == NULL);

  MySet copy = mySetCopy(set);
  ASSERT_TEST(mySetRemove(copy, hello) == MY_SET_SUCCESS);

  ASSERT_TEST(strcmp(goodDay, mySetGetFirst(set)) == 0);
  ASSERT_TEST(strcmp(goodbye, mySetGetNext(set)) == 0);
  ASSERT_TEST(strcmp(hello, mySetGetNext(set)) == 0);
  ASSERT_TEST(mySetGetNext(set) == NULL);

  ASSERT_TEST(strcmp(goodDay, mySetGetFirst(copy)) == 0);
  ASSERT_TEST(strcmp(goodbye, mySetGetNext(copy)) == 0);
  ASSERT_TEST(mySetGetNext(copy) == NULL);

  ASSERT_TEST(mySetClear(copy) == MY_SET_SUCCESS);
  ASSERT_TEST(mySetGetFirst(copy) == NULL);

  MySet filtered = mySetFilter(set, longerThanTen);
  ASSERT_TEST(strcmp(hello, mySetGetFirst(filtered)) == 0);
  ASSERT_TEST(mySetGetNext(filtered) == NULL);

  mySetDestroy(filtered);
  mySetDestroy(set);
  mySetDestroy(copy);

  return true;
}


#define MY_SET_TEST_ALLOCATION(type, variable, error) \
	do { \
		if(NULL == (variable = (type*)malloc(sizeof(type)))) { \
			return error; \
		} \
	}while(false);

#define INT(e) (*(int*)(e))

static MySetElement copyInt(const MySetElement element) {
	if (!element) {
		return NULL;
	}
	MySetElement copy;
	MY_SET_TEST_ALLOCATION(int, copy, NULL);
	INT(copy) = INT(element);
	return copy;
}

static void freeInt(MySetElement element) {
	free(element);
}

static int compareInt(const MySetElement a, const MySetElement b) {
	return INT(a) - INT(b);
}

static bool oddIntFilter(const MySetElement a) {
	return INT(a) % 2 == 1;
}

static bool testMySetCopy() {
	ASSERT_TEST(mySetCopy(NULL) == NULL);
	//copy empty set
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	ASSERT_TEST(set != NULL);
	MySet copy = mySetCopy(set);
	ASSERT_TEST(copy != NULL);
	ASSERT_TEST(mySetGetSize(copy) == 0);
	mySetDestroy(set);
	//adding "1"
	int *one;
	MY_SET_TEST_ALLOCATION(int, one, (mySetDestroy(copy), false));
	*one = 1;
	ASSERT_TEST(mySetAdd(copy, one) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetFirst(copy) != NULL);
	set = copy;
	//copy set with "1"
	copy = mySetCopy(set);
	mySetDestroy(set);
	ASSERT_TEST(mySetGetSize(copy) == 1);
	ASSERT_TEST(mySetAdd(copy, one) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetCurrent(copy) == NULL);
	freeInt(one);
	mySetDestroy(copy);

	return true;
}

static bool testMySetGetSize() {
	ASSERT_TEST(mySetGetSize(NULL) == -1);
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	ASSERT_TEST(set != NULL);
	ASSERT_TEST(mySetGetSize(set) == 0);
	int *one, *two;
	MY_SET_TEST_ALLOCATION(int, one, (mySetDestroy(set), false));
	MY_SET_TEST_ALLOCATION(int, two, (mySetDestroy(set), freeInt(one), false));
	*one = 1;
	*two = 2;

	ASSERT_TEST(mySetAdd(set, one) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetAdd(set, one) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetAdd(set, two) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 2);
	ASSERT_TEST(mySetRemove(set, one) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetRemove(set, one) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 1);
	MySet copy = mySetCopy(set);
	mySetDestroy(set);
	ASSERT_TEST(copy != NULL);
	ASSERT_TEST(mySetGetSize(copy) == 1);
	ASSERT_TEST(mySetAdd(copy, one) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(copy) == 2);
	int *extracted = mySetExtract(copy, two);
	ASSERT_TEST(extracted != NULL && *extracted == 2);
	ASSERT_TEST(mySetGetSize(copy) == 1);
	ASSERT_TEST(mySetClear(copy) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(copy) == 0);


	freeInt(one);
	freeInt(two);
	freeInt(extracted);
	mySetDestroy(copy);
	return true;
}

static bool testMySetForeach() {
	// values
	const int VALUES_NUMBER = 7;
	int* values[VALUES_NUMBER];
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		values[i] = (int*)malloc(sizeof(int));
		if (values[i] == NULL) {
			while (i) {
				freeInt(values[--i]);
			}
		}
		*values[i] = i;
	}

	ASSERT_TEST(mySetGetFirst(NULL) == NULL);
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	ASSERT_TEST(mySetGetFirst(set) == NULL);
	ASSERT_TEST(mySetAdd(set, values[5]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[2]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[1]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[4]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == VALUES_NUMBER);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetAdd(set, values[4]) == MY_SET_ITEM_ALREADY_EXISTS);

	int index = 1;
	MY_SET_FOREACH(int *, value, set) {
		ASSERT_TEST(index < VALUES_NUMBER);
		ASSERT_TEST(compareInt(value, values[index]) == 0);

		//check consistency
		mySetDestroy(mySetCopy(set));
		ASSERT_TEST(mySetGetSize(set) == VALUES_NUMBER-1);
		ASSERT_TEST(mySetIsIn(set, values[index]));
		mySetDestroy(mySetFilter(set, oddIntFilter));
		ASSERT_TEST(compareInt(mySetGetCurrent(set), values[index]) == 0);
		ASSERT_TEST(mySetGetCurrent(set) == value);

		++index;
	}
	ASSERT_TEST(index == VALUES_NUMBER);

	mySetDestroy(set);
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		freeInt(values[i]);
	}
	return true;
}

static bool testMySetAdd() {
	// values
	const int VALUES_NUMBER = 7;
	int* values[VALUES_NUMBER];
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		values[i] = (int*)malloc(sizeof(int));
		if (values[i] == NULL) {
			while (i) {
				freeInt(values[--i]);
			}
		}
		*values[i] = i;
	}

	ASSERT_TEST(mySetAdd(NULL, values[0]) == MY_SET_NULL_ARGUMENT);
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	ASSERT_TEST(mySetAdd(set, NULL) == MY_SET_NULL_ARGUMENT);
	ASSERT_TEST(set != NULL);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetAdd(set, values[2]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 2);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 3);
	ASSERT_TEST(mySetAdd(set, values[4]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// reinsert first
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// reinsert middle
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// reinsert last
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert first
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert middle
	ASSERT_TEST(mySetRemove(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert last
	ASSERT_TEST(mySetRemove(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert first
	freeInt(mySetExtract(set, values[0]));
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert middle
	freeInt(mySetExtract(set, values[3]));
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	// remove and reinsert last
	freeInt(mySetExtract(set, values[6]));
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);

	ASSERT_TEST(mySetClear(set) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);


	mySetDestroy(set);
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		freeInt(values[i]);
	}
	return true;
}

static bool testMySetRemove() {
	// values
	const int VALUES_NUMBER = 7;
	int* values[VALUES_NUMBER];
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		values[i] = (int*)malloc(sizeof(int));
		if (values[i] == NULL) {
			while (i) {
				freeInt(values[--i]);
			}
		}
		*values[i] = i;
	}
	MySet set = mySetCreate(copyInt, freeInt, compareInt);


	ASSERT_TEST(mySetRemove(NULL, values[0]) == MY_SET_NULL_ARGUMENT);
	ASSERT_TEST(mySetRemove(set, NULL) == MY_SET_NULL_ARGUMENT);
	ASSERT_TEST(mySetGetSize(set) == 0);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 0);
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[4]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[1]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[5]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[2]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 7);
	ASSERT_TEST(mySetRemove(set, NULL) == MY_SET_NULL_ARGUMENT);
	ASSERT_TEST(mySetGetSize(set) == 7);
	ASSERT_TEST(mySetRemove(set, values[4]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 6);
	ASSERT_TEST(mySetRemove(set, values[4]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 6);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 5);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 5);
	ASSERT_TEST(mySetRemove(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetRemove(set, values[6]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetRemove(set, values[4]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetRemove(set, values[0]) == MY_SET_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(mySetGetSize(set) == 4);
	ASSERT_TEST(mySetRemove(set, values[1]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 3);
	ASSERT_TEST(mySetRemove(set, values[2]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 2);
	ASSERT_TEST(mySetRemove(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 1);
	ASSERT_TEST(mySetRemove(set, values[5]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetGetSize(set) == 0);

	mySetDestroy(set);
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		freeInt(values[i]);
	}
	return true;
}

static bool testMySetClear() {
	ASSERT_TEST(mySetClear(NULL) == MY_SET_NULL_ARGUMENT);
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	for (int i = 0; i < 2; ++i) {
		ASSERT_TEST(mySetGetSize(set) == 0);
		const int VALUES_NUMBER = 7;
		for (int i = 0; i < VALUES_NUMBER; ++i) {
			int *value;
			MY_SET_TEST_ALLOCATION(int, value, (mySetDestroy(set),false));
			*value = i;
			ASSERT_TEST(mySetAdd(set, value) == MY_SET_SUCCESS);
			freeInt(value);
		}
		ASSERT_TEST(mySetGetSize(set) == VALUES_NUMBER);
		ASSERT_TEST(mySetClear(set) == MY_SET_SUCCESS);
		ASSERT_TEST(mySetGetSize(set) == 0);
	}
	mySetDestroy(set);
	return true;
}

static bool testMySetCreate() {
	MySet set;
	set = mySetCreate(copyInt, freeInt, compareInt);
	ASSERT_TEST(mySetGetCurrent(set) == NULL);
	ASSERT_TEST(mySetGetFirst(set) == NULL);
	ASSERT_TEST(set!=NULL);
	mySetDestroy(set);
	set = mySetCreate(NULL, freeInt, compareInt);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(copyInt, NULL, compareInt);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(copyInt, freeInt, NULL);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(NULL, NULL, compareInt);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(NULL, freeInt, NULL);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(copyInt, NULL, NULL);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	set = mySetCreate(NULL, NULL, NULL);
	ASSERT_TEST(set==NULL);
	mySetDestroy(set);
	return true;
}

static bool testMySetDestroy() {
	MySet set = mySetCreate(copyInt, freeInt, compareInt);
	const int VALUES_NUMBER = 7;
	int* values[VALUES_NUMBER];
	for (int i = 0; i < VALUES_NUMBER; ++i) {
		values[i] = (int*)malloc(sizeof(int));
		if (values[i] == NULL) {
			while (i) {
				freeInt(values[--i]);
			}
		}
		*values[i] = i;
	}
	ASSERT_TEST(mySetAdd(set, values[0]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[3]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[4]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[1]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[5]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[2]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetAdd(set, values[6]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetRemove(set, values[1]) == MY_SET_SUCCESS);
	ASSERT_TEST(mySetRemove(set, values[5]) == MY_SET_SUCCESS);
	mySetDestroy(set);

	mySetDestroy(NULL);
	return true;
}

int main() {
	RUN_TEST(testMySetExample);
	RUN_TEST(testMySetCopy);
	RUN_TEST(testMySetGetSize);
	RUN_TEST(testMySetForeach);
	RUN_TEST(testMySetAdd);
	RUN_TEST(testMySetRemove);
	RUN_TEST(testMySetClear);

	RUN_TEST(testMySetCreate);
	RUN_TEST(testMySetDestroy);
	//RUN_TEST(testMySetIsIn);
	//RUN_TEST(testMySetExtract);
	//RUN_TEST(testMySetFilter);
	return 0;
}

