/*
 * memcache_test.c
 *
 *  Created on: Nov 26, 2015
 *      Author: tamer.mour
 */

#include "test_utilities.h"
#include <stdlib.h>
#include "../memcache.h"
#include <string.h>

#define ASSERT_TRUE(x) ASSERT_TEST(x);
#define ASSERT_EQUAL(x,y) ASSERT_TEST((x) == (y));
#define ASSERT_EQUAL_STR(x,y) ASSERT_TEST(strcmp((x),(y)) == 0);
#define ASSERT_SUCCESS(x) ASSERT_EQUAL((x), MEMCACHE_SUCCESS)
#define ASSERT_NOT_NULL(x) ASSERT_TRUE(x)
#define ASSERT_NULL(x) ASSERT_TRUE(!x)

/** Functions to be used for checking blocks' status */
static bool checkBlock(void* ptr, int size, char mod, const char* const username, char* data) {
  char* block = ptr;
  block -= (sizeof(int) + 1 + 8 + 3);
  ASSERT_EQUAL(size, *(int*)block);
  block += sizeof(int);
  ASSERT_EQUAL(*block, '\0');
  block += 1;
  ASSERT_EQUAL(*block, mod);
  block += 1;
  ASSERT_EQUAL_STR(username, block);
  block += strlen(username) + 1;
  ASSERT_EQUAL(*block, '\0');
  block += 1;
  if (data != NULL)
    ASSERT_EQUAL_STR(data, block);
  return true;
}

bool memCacheExampleTest() {
  MemCache memcache;
  ASSERT_NOT_NULL(memcache = memCacheCreate());

  ASSERT_SUCCESS(memCacheAddUser(memcache, "jbond007", 1000));
  ASSERT_SUCCESS(memCacheAddUser(memcache, "j0walker", 500));

  char *ptr1 = NULL;
  ASSERT_NOT_NULL(ptr1 = memCacheAllocate(memcache, "jbond007", 10));

  char *ptr2 = NULL;
  ASSERT_NOT_NULL(ptr2 = memCacheAllocate(memcache, "j0walker", 50));

  strcpy(ptr1, "A string.");
  strcpy(ptr2, "Some other string.");

  ASSERT_SUCCESS(memCacheSetBlockMod(memcache, "jbond007", ptr1, 'G'));

  ASSERT_TRUE(checkBlock(ptr1, 10, 'G', "jbond007", "A string."));
  ASSERT_TRUE(checkBlock(ptr2, 50, 'U', "j0walker", "Some other string."));

  ASSERT_SUCCESS(memCacheTrust(memcache, "jbond007", "j0walker"));

  ASSERT_EQUAL(memCacheFree(memcache, "jbond007", ptr2),
      MEMCACHE_PERMISSION_DENIED);
  ASSERT_SUCCESS(memCacheFree(memcache, "j0walker", ptr1));

  ASSERT_NOT_NULL(memCacheGetFirstFreeBlock(memcache));
  ASSERT_NOT_NULL(memCacheGetFirstAllocatedBlock(memcache));

  memCacheDestroy(memcache);
  return true;
}

int main() {
  RUN_TEST(memCacheExampleTest);
  return 0;
}


