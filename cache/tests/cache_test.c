#include "test_utilities.h"
#include <stdlib.h>
#include "../cache.h"
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
  return *(unsigned char*)element;
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
  
  char* temp_ptr;
  temp_ptr = cacheExtractElementByKey(cache, 0);
  ASSERT_TEST(strcmp(temp_ptr, "") == 0);
  ASSERT_TEST(cacheIsIn(cache, empty_string) == false);
  ASSERT_TEST(cacheFreeElement(cache, goodbye) == CACHE_SUCCESS);
  ASSERT_TEST(cacheIsIn(cache, goodbye) == false);

  ASSERT_TEST(cacheClear(cache) == CACHE_SUCCESS);
  ASSERT_TEST(setGetFirst(cacheGetFirst(cache)) == NULL);

  free(temp_ptr);
  cacheDestroy(cache);
  return true;
}

static bool testCacheCreate(void) {
	ASSERT_TEST(!cacheCreate(0, freeString, copyString, compareStrings, getFirstLetter));
	ASSERT_TEST(!cacheCreate(256, NULL, copyString, compareStrings, getFirstLetter));
	ASSERT_TEST(!cacheCreate(256, freeString, NULL, compareStrings, getFirstLetter));
	ASSERT_TEST(!cacheCreate(256, freeString, copyString, NULL, getFirstLetter));
	ASSERT_TEST(!cacheCreate(256, freeString, copyString, compareStrings, NULL));

	Cache cache = cacheCreate(1, freeString, copyString, compareStrings, getFirstLetter);
	ASSERT_TEST(cache != NULL);
	cacheDestroy(cache);
	return true;
}

static bool testCachePush() {
	//strings
	char * toFirstCell = "";
	char * ramones = "Ramones";
	char toLastCell[] = {(char)254, '\0'};
	char outOfRange[] = {(char)255, '\0'};

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

static bool testCacheFreeElement(void) {
	Cache cache = cacheCreate(255, freeString, copyString, compareStrings, getFirstLetter);

	char outOfRange[2] = { (char)255, '\0' };
	char *notInCache = "Rock";
	char *doubled = "Louna";
	char * elements[] = {
			"Ramones",
			"Lumen",
			"Louna",
			"Arch Enemy"
			"Linkin park",
	};
	const int ELEMENTS_SIZE = sizeof(elements) / sizeof(*elements);

	for (int i = 0; i < ELEMENTS_SIZE; ++i) {
		ASSERT_TEST(cachePush(cache, elements[i]) == CACHE_SUCCESS);
	}
	ASSERT_TEST(cachePush(cache, outOfRange) == CACHE_OUT_OF_RANGE);
	ASSERT_TEST(cachePush(cache, doubled) == CACHE_ITEM_ALREADY_EXISTS);

	ASSERT_TEST(cacheFreeElement(cache, outOfRange) == CACHE_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(cacheFreeElement(cache, notInCache) == CACHE_ITEM_DOES_NOT_EXIST);
	for (int i = ELEMENTS_SIZE-1; i >= 0; --i) {
		ASSERT_TEST(cacheFreeElement(cache, elements[i]) == CACHE_SUCCESS);
	}
	ASSERT_TEST(cacheFreeElement(cache, doubled) == CACHE_ITEM_DOES_NOT_EXIST);
	ASSERT_TEST(cacheFreeElement(NULL, doubled) == CACHE_NULL_ARGUMENT);
	ASSERT_TEST(cacheFreeElement(NULL, NULL) == CACHE_NULL_ARGUMENT);

	cacheDestroy(cache);
	return true;
}

static bool testCacheExtractElementByKey() {
	char * andykaufman = "andykaufman";
	char * agonist = "agonist";
	char * nightwish = "nightwish";

	Cache cache = cacheCreate(256, freeString, copyString, compareStrings, getFirstLetter);
	ASSERT_TEST(cache != NULL);

	ASSERT_TEST(cachePush(cache, nightwish) == CACHE_SUCCESS);
	ASSERT_TEST(cachePush(cache, nightwish) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, nightwish));
	ASSERT_TEST(cachePush(cache, andykaufman) == CACHE_SUCCESS);
	ASSERT_TEST(cachePush(cache, andykaufman) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, andykaufman));
	ASSERT_TEST(cachePush(cache, agonist) == CACHE_SUCCESS);
	ASSERT_TEST(cachePush(cache, agonist) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, agonist));

	const int SIZE = 3;
	char * elements[SIZE];
	int i = 0;
	CACHE_FOREACH(set, cache) {
		SET_FOREACH(char *, str, set) {
			elements[i++] = str;
		}
	}
	ASSERT_TEST(i == SIZE);

	char * ex1 = cacheExtractElementByKey(cache, getFirstLetter(andykaufman));
	char * ex2 = cacheExtractElementByKey(cache, getFirstLetter(agonist));
	ASSERT_TEST((ex1 == elements[0] && ex2 == elements[1]) ||
			(ex2 == elements[0] && ex1 == elements[1]));
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(nightwish)) == elements[2]);
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(andykaufman)) == NULL);
	ASSERT_TEST(cacheExtractElementByKey(cache, getFirstLetter(nightwish)) == NULL);

	free(elements[0]);
	free(elements[1]);
	free(elements[2]);

	cacheDestroy(cache);

	return true;
}

static bool testCacheIsIn(void) {
	Cache cache = cacheCreate(255, freeString, copyString, compareStrings, getFirstLetter);
	ASSERT_TEST(cache != NULL);

	char outOfRange[2] = { (char)255, '\0' };
	char *notInCache = "Rock";
	char *doubled = "Louna";
	char * elements[] = {
			"Ramones",
			"Lumen",
			"Louna",
			"Arch Enemy",
			"Linkin park"
	};

	const int ELEMENTS_SIZE = sizeof(elements) / sizeof(*elements);
	for (int i = 0; i < ELEMENTS_SIZE; ++i) {
		ASSERT_TEST(!cacheIsIn(cache, elements[i]));
		ASSERT_TEST(cachePush(cache, elements[i]) == CACHE_SUCCESS);
		ASSERT_TEST(cacheIsIn(cache, elements[i]));
	}
	ASSERT_TEST(!cacheIsIn(cache, notInCache));
	ASSERT_TEST(!cacheIsIn(NULL, notInCache));
	ASSERT_TEST(cachePush(cache, outOfRange) == CACHE_OUT_OF_RANGE);
	ASSERT_TEST(!cacheIsIn(cache, outOfRange));
	ASSERT_TEST(cachePush(cache, doubled) == CACHE_ITEM_ALREADY_EXISTS);
	ASSERT_TEST(cacheIsIn(cache, doubled));

	for (int i = 0; i < ELEMENTS_SIZE; ++i) {
		ASSERT_TEST(cacheIsIn(cache, elements[i]));
		ASSERT_TEST(cacheFreeElement(cache, elements[i]) == CACHE_SUCCESS);
		ASSERT_TEST(!cacheIsIn(cache, elements[i]));
	}
	ASSERT_TEST(cacheFreeElement(cache, notInCache) == CACHE_ITEM_DOES_NOT_EXIST);

	cacheDestroy(cache);
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

	const int TEST_CACHE_FOREACH_ELEMENTS = 100;
	int counted[TEST_CACHE_FOREACH_ELEMENTS];

	for (int i = 0; i < TEST_CACHE_FOREACH_ELEMENTS; ++i) {
		counted[i] = 0;
		ASSERT_TEST(cachePush(cache, &i) == CACHE_SUCCESS);
		ASSERT_TEST(cachePush(cache, &i) == CACHE_ITEM_ALREADY_EXISTS);
		ASSERT_TEST(cacheIsIn(cache, &i));
	}

	ASSERT_TEST(cacheGetCurrent(NULL) == NULL);
	ASSERT_TEST(cacheGetFirst(NULL) == NULL);
	ASSERT_TEST(cacheGetNext(NULL) == NULL);
	ASSERT_TEST(cacheGetCurrent(cache) == NULL);

	CACHE_FOREACH(set, cache) {
		SET_FOREACH(int*, it, set) {
			++counted[INT(it)];
		}

		//should not influence iterator
		for (int i = 0; i < TEST_CACHE_FOREACH_ELEMENTS; ++i) {
			ASSERT_TEST(cacheIsIn(cache, &i));
		}
	}
	ASSERT_TEST(cacheGetNext(cache) == NULL);
	ASSERT_TEST(cacheGetCurrent(cache) == NULL);


	for (int i = 0; i < TEST_CACHE_FOREACH_ELEMENTS; ++i) {
		ASSERT_TEST(counted[i] == 1);
	}


	cacheDestroy(cache);
	return true;
}

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	RUN_TEST(testCacheExample);
	RUN_TEST(testCacheCreate);
	RUN_TEST(testCachePush);
	RUN_TEST(testCacheFreeElement);
	RUN_TEST(testCacheExtractElementByKey);
	RUN_TEST(testCacheIsIn);
	RUN_TEST(testCacheForeach);
	return 0;
}

