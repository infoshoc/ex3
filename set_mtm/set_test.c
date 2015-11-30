#include "test_utilities.h"
#include <stdlib.h>
#include "../set.h"
#include <string.h>

/** Function to be used for copying an int into the set */
static SetElement copyString(SetElement str) {
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
static void freeString(SetElement str) {
	free(str);
}

/** Function for comparing to elements in the set */
static int compareStrings(SetElement element1, SetElement element2) {
	return strcmp(element1, element2);
}

static bool longerThanTen(SetElement element) {
  return strlen(element) > 10;
}

bool testSetExample() {
  char* hello = "Hello world!";
  char* helloAgain = "Hello world!";
  char* goodbye = "Goodbye!";
  char* goodDay = "Good day!";

  Set set = setCreate(copyString, freeString, compareStrings);
  ASSERT_TEST(set != NULL);
  ASSERT_TEST(setGetSize(set) == 0);
  ASSERT_TEST(setIsIn(set, hello) == false);
  ASSERT_TEST(setAdd(set, hello) == SET_SUCCESS);
  ASSERT_TEST(setGetSize(set) == 1);
  ASSERT_TEST(setIsIn(set, helloAgain) == true);
  ASSERT_TEST(setAdd(set, hello) == SET_ITEM_ALREADY_EXISTS);
  ASSERT_TEST(setGetSize(set) == 1);

  ASSERT_TEST(setAdd(set, goodbye) == SET_SUCCESS);
  ASSERT_TEST(setAdd(set, goodDay) == SET_SUCCESS);

  ASSERT_TEST(strcmp(goodDay, setGetFirst(set)) == 0);
  ASSERT_TEST(strcmp(goodbye, setGetNext(set)) == 0);
  ASSERT_TEST(strcmp(hello, setGetNext(set)) == 0);
  ASSERT_TEST(setGetNext(set) == NULL);

  Set copy = setCopy(set);
  ASSERT_TEST(setRemove(copy, hello) == SET_SUCCESS);

  ASSERT_TEST(strcmp(goodDay, setGetFirst(set)) == 0);
  ASSERT_TEST(strcmp(goodbye, setGetNext(set)) == 0);
  ASSERT_TEST(strcmp(hello, setGetNext(set)) == 0);
  ASSERT_TEST(setGetNext(set) == NULL);

  ASSERT_TEST(strcmp(goodDay, setGetFirst(copy)) == 0);
  ASSERT_TEST(strcmp(goodbye, setGetNext(copy)) == 0);
  ASSERT_TEST(setGetNext(copy) == NULL);

  ASSERT_TEST(setClear(copy) == SET_SUCCESS);
  ASSERT_TEST(setGetFirst(copy) == NULL);

  Set filtered = setFilter(set, longerThanTen);
  ASSERT_TEST(strcmp(hello, setGetFirst(filtered)) == 0);
  ASSERT_TEST(setGetNext(filtered) == NULL);

  setDestroy(filtered);
  setDestroy(set);
  setDestroy(copy);

  return true;
}

int main() {
	RUN_TEST(testSetExample);
	return 0;
}

