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

int main() {
	RUN_TEST(testMySetExample);
	return 0;
}

