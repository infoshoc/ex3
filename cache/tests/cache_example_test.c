#include "test_utilities.h"
#include <stdlib.h>
#include "../cache.h"
#include "../set.h"
#include <string.h>

/**
* This example program tests the generic cache ADT by creating
* and manipulating a cache of strings with a container of size
* 256, where the i'th cell in the container contains all strings
* that start with the i'th character in the ASCII table.
*/


/** Function to be used for copying a string into the cache */
static CacheElement copyString(CacheElement str) {
  if (!str) {
    return NULL;
  }
  char* copy = malloc(strlen(str) + 1);
  if (!copy) {
    return NULL;
  }
  return strcpy(copy, str);
}

/** Function to be used by the cache for freeing elements */
static void freeString(CacheElement str) {
  free(str);
}

/** Function for comparing two elements in the cache */
static int compareStrings(CacheElement element1, CacheElement element2) {
  return strcmp(element1, element2);
}

/** Function to compute the key for an element in the cache */
static int getFirstLetter(CacheElement element) {
  return *(char*)element;
}

bool testCacheExample() {
  char* empty_string = "";
  char* hello = "Hello world!";
  char* helloAgain = "Hello world!";
  char* goodbye = "Goodbye!";
  char* goodDay = "Good day!";

  Cache cache = cacheCreate(256, freeString, copyString, compareStrings, getFirstLetter);
  ASSERT_TEST(cache != NULL);
  ASSERT_TEST(cacheIsIn(cache, hello) == false);
  ASSERT_TEST(cachePush(cache, hello) == CACHE_SUCCESS);
  ASSERT_TEST(cacheIsIn(cache, helloAgain) == true);
  ASSERT_TEST(cachePush(cache, hello) == CACHE_ITEM_ALREADY_EXISTS);

  ASSERT_TEST(cachePush(cache, goodbye) == CACHE_SUCCESS);
  ASSERT_TEST(cachePush(cache, goodDay) == CACHE_SUCCESS);
  ASSERT_TEST(cachePush(cache, empty_string) == CACHE_SUCCESS);

  ASSERT_TEST(cacheIsIn(cache, empty_string) == true);
  ASSERT_TEST(cacheIsIn(cache, goodbye) == true);
  ASSERT_TEST(strcmp(cacheExtractElementByKey(cache, 0), "") == 0);
  ASSERT_TEST(cacheIsIn(cache, empty_string) == false);
  ASSERT_TEST(cacheFreeElement(cache, goodbye) == CACHE_SUCCESS);
  ASSERT_TEST(cacheIsIn(cache, goodbye) == false);

  ASSERT_TEST(cacheClear(cache) == CACHE_SUCCESS);
  ASSERT_TEST(setGetFirst(cacheGetFirst(cache)) == NULL);

  cacheDestroy(cache);
  return true;
}

static bool testCachePush() {
	//strings
	const char * const toFirstCell = "";
	const char * const ramones = "Ramones";
	const char * const toLastCell = {(char)254, '\0'};
	const char * const outOfRange = {(char)255, '\0'};

	Cache cache = cacheCreate(255, freeString, copyString, compareStrings, getFirstLetter);
	ASSERT_TEST(cache != NULL);

	ASSERT_TEST(cachePush(NULL, toFirstCell) == CACHE_NULL_ARGUMENT);
	ASSERT_TEST(cachePush(cache, NULL) == CACHE_NULL_ARGUMENT);
	ASSERT_TEST(cachePush(cache, toFirstCell) == CACHE_SUCCESS);
	ASSERT_TEST(cacheIsIn(cache, toFirstCell));
	ASSERT_TEST(cacheIsIn(cache, toFirstCell));
	ASSERT_TEST(cachePush(cache, ramones) == CACHE_SUCCESS);
	ASSERT_TEST(cacheIsIn(cache, ramones));
	ASSERT_TEST(cachePush(cache, toLastCell) == CACHE_SUCCESS);
	ASSERT_TEST(cacheIsIn(cache, toLastCell));
	ASSERT_TEST(cachePush(cache, outOfRange) == CACHE_OUT_OF_RANGE);
	ASSERT_TEST(!cacheIsIn(cache, outOfRange));
	ASSERT_TEST(cachePush(cache, ramones) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cachePush(cache, toFirstCell) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cachePush(cache, toLastCell) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cachePush(cache, outOfRange) == CACHE_OUT_OF_RANGE);

	char * toFirstCellCopy = cacheExtractElementByKey(cache, getFirstLetter(toFirstCell));
	char * ramonesCopy = cacheExtractElementByKey(cache, getFirstLetter(ramones));
	char * toLastCellCopy = cacheExtractElementByKey(cache, getFirstLetter(toLastCell));
	char * outOfRangeCopy = cacheExtractElementByKey(cache, getFirstLetter(outOfRange));

	ASSERT_TEST(!strcmp(toFirstCell, toFirstCellCopy));
	ASSERT_TEST(!strcmp(ramones, ramonesCopy));
	ASSERT_TEST(!strcmp(toLastCell, toLastCellCopy));
	ASSERT_TEST(outOfRangeCopy == NULL);

	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(toFirstCell)) == NULL);
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(ramones)) == NULL);
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(toLastCell)) == NULL);

	freeString(toFirstCellCopy);
	freeString(ramonesCopy);
	freeString(toLastCellCopy);
	cacheDestroy(cache);

	return true;
}

static bool testCacheExtractElementByKey() {
	const char * const andykaufman = "andykaufman";
	const char * const nightwish = "nightwish";

	Cache cache = cacheCreate(256, freeString, copyString, compareStrings, getFirstLetter);
	ASSERT_TEST(cache != NULL);

	ASSERT_TEST(cachePush(cache, nightwish) == CACHE_SUCCESS);
	ASSERT_TEST(cachePush(cache, nightwish) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, nightwish));
	ASSERT_TEST(cachePush(cache, andykaufman) == CACHE_SUCCESS);
	ASSERT_TEST(cachePush(cache, andykaufman) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, andykaufman));

	const int SIZE = 2;
	char * elements[SIZE];
	int i = 0;
	CACHE_FOREACH(set, cache) {
		SET_FOREACH(char *, str, set) {
			elements[i++] = str;
		}
	}
	ASSERT_TEST(i == SIZE);

	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(andykaufman) == elements[0]));
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(nightwish) == elements[1]));
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(andykaufman) == NULL));
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(nightwish) == NULL));

	free(elements[0]);
	free(elements[1]);

	return true;
}

#define INT(x) (*(int*)(x))

static CacheElement copyInt(CacheElement element) {
	CacheElement copy = (CacheElement)malloc(sizeof(int));
	if (!copy) {
		return NULL;
	}
	INT(copy) = INT(element);
	return copy;
}

static void freeInt(CacheElement element) {
	free(element);
}

static int compareInt(CacheElement int1, CacheElement int2) {
	return INT(int1) - INT(int2);
}
const int BASE = 10;
static int getLastDigit(CacheElement element) {
	return INT(element) % BASE;
}

static bool testCacheForeach() {

	Cache cache = cacheCreate(BASE, freeInt, copyInt, compareInt, getLastDigit);
	ASSERT_TEST(cache != NULL);

	const int ELEMENTS = 100;
	int counted[ELEMENTS] = {};

	for (int i = 0; i < ELEMENTS; ++i) {
		ASSERT_TEST(cachePush(cache, &i) == CACHE_SUCCESS);
		ASSERT_TEST(cachePush(cache, &i) == CACHE_ITEM_ALREADY_EXISTS);
		ASSERT_TEST(cacheIsIn(cache, &i));
	}

	CACHE_FOREACH(set, cache) {
		SET_FOREACH(int*, it, set) {
			++counted[INT(it)];
		}

		//should not influence iterator
		for (int i = 0; i < ELEMENTS; ++i) {
			ASSERT_TEST(cacheIsIn(cache, &i));
		}
	}


	for (int i = 0; i < ELEMENTS; ++i) {
		ASSERT_TEST(counted[i] == 1);
	}


	cacheDestroy(cache);
	return true;
}

int main() {
	RUN_TEST(testCacheExample);
	RUN_TEST(testCachePush);
	RUN_TEST(testCacheExtractElementByKey);
	return 0;
}

