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

int main() {
	RUN_TEST(testCacheExample);
	return 0;
}

