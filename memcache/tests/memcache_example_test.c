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

static bool memCacheDestroyTest(void) {
	// is NULL stable
	memCacheDestroy(NULL);

	// use all possible structures to detect leak
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);
	const int ALLOCATED_CACHE_SIZE = (1<<16);
	const int FREE_CACHE_SIZE = 256;
	const int ALL_POSSIBLE_ALLOCATED_BLOCKS_SUM = 2147516416;
	const int ALL_POSSIBLE_FREE_BLOCKS_SUM = 3341;
	// add users
	const char * const username1 = "LepsGena";
	const char * const username2 = "Coldplay";
	ASSERT_EQUAL(memCacheAddUser(memcache, username1, ALL_POSSIBLE_ALLOCATED_BLOCKS_SUM), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, username2, ALL_POSSIBLE_FREE_BLOCKS_SUM), MEMCACHE_SUCCESS);

	//allocate memory
	void *user1Memory[ALLOCATED_CACHE_SIZE];
	void *user2Memory[FREE_CACHE_SIZE];
	for (int size = 1; size <= ALLOCATED_CACHE_SIZE; ++size) {
		user1Memory[size-1] = memCacheAllocate(memcache, username1, size);
		ASSERT_NOT_NULL(user1Memory[size-1]);
	}
	for (int size = 1; size <= FREE_CACHE_SIZE; ++size) {
		user2Memory[size-1] = memCacheAllocate(memcache, username2, size);
		ASSERT_NOT_NULL(user2Memory[size-1]);
	}

	//user1 and user2 trust each other
	ASSERT_EQUAL(memCacheTrust(memcache, username1, username2), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheTrust(memcache, username2, username1), MEMCACHE_SUCCESS);

	//user1 frees all memory user2 has, while latter is sleeping
	for (int size = 1; size <= FREE_CACHE_SIZE; ++size) {
		ASSERT_EQUAL(memCacheFree(memcache, username1, user2Memory[size-1]), MEMCACHE_SUCCESS);
	}

	//user2 doesn't trusts to user1 anymore
	ASSERT_EQUAL(memCacheUntrust(memcache, username2, username1), MEMCACHE_SUCCESS);

	//start some iterations
	ASSERT_NOT_NULL(memCacheGetFirstFreeBlock(memcache));
	ASSERT_NOT_NULL(memCacheGetFirstAllocatedBlock(memcache));

	memCacheDestroy(memcache);

	return true;
}

static bool memCacheSetBlockModTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	const char * const user1 = "gammaray";
	const char * const user2 = "scorpion";
	const char * const user3 = "amaranth";
	const char * const user4 = "nightwis";

	// add users to system
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, 100), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user2, 100), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user3, 100), MEMCACHE_SUCCESS);

	// relations
	ASSERT_EQUAL(memCacheTrust(memcache, user1, user2), MEMCACHE_SUCCESS);

	// allocations
	void *block1 = memCacheAllocate(memcache, user1, 10);
	ASSERT_NOT_NULL(block1);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	void *block2 = memCacheAllocate(memcache, user2, 23);
	ASSERT_NOT_NULL(block2);
	ASSERT_TRUE(checkBlock(block2, 23, 'U', user2, NULL));

	// releasing
	ASSERT_EQUAL(memCacheFree(memcache, user2, block2), MEMCACHE_SUCCESS);


	ASSERT_EQUAL(memCacheSetBlockMod(NULL, NULL, NULL, 'B'), MEMCACHE_NULL_ARGUMENT);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, NULL, NULL, 'B'), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user4, NULL, 'B'), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, NULL, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1+1, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block2, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block2, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block1, 'B'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1, 'B'), MEMCACHE_INVALID_ARGUMENT);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1, 'U'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));

	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1, 'G'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block1, 10, 'G', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block1, 'A'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'G', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1, 'A'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block1, 10, 'A', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user3, block1, 'U'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'A', user1, NULL));

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheUntrustTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	const char * const user1 = "gammaray";
	const char * const user2 = "scorpion";
	const char * const user3 = "amaranth";
	const char * const user4 = "nightwis";

	// add users to system
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, 100), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user2, 100), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user3, 100), MEMCACHE_SUCCESS);

	// trusting
	ASSERT_EQUAL(memCacheTrust(memcache, user1, user2), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheTrust(memcache, user2, user1), MEMCACHE_SUCCESS);

	// allocations
	void *block1U1 = memCacheAllocate(memcache, user1, 23);
	ASSERT_NOT_NULL(block1U1);
	ASSERT_TRUE(checkBlock(block1U1, 23, 'U', user1, NULL));
	void *block1G1 = memCacheAllocate(memcache, user1, 42);
	ASSERT_NOT_NULL(block1G1);
	ASSERT_TRUE(checkBlock(block1G1, 42, 'U', user1, NULL));
	void *block1A1 = memCacheAllocate(memcache, user1, 13);
	ASSERT_NOT_NULL(block1A1);
	ASSERT_TRUE(checkBlock(block1A1, 13, 'U', user1, NULL));
	void *block2U1 = memCacheAllocate(memcache, user2, 23);
	ASSERT_NOT_NULL(block2U1);
	ASSERT_TRUE(checkBlock(block2U1, 23, 'U', user2, NULL));
	void *block2G1 = memCacheAllocate(memcache, user2, 42);
	ASSERT_NOT_NULL(block2G1);
	ASSERT_TRUE(checkBlock(block2G1, 42, 'U', user2, NULL));
	void *block2A1 = memCacheAllocate(memcache, user2, 13);
	ASSERT_NOT_NULL(block2A1);
	ASSERT_TRUE(checkBlock(block2A1, 13, 'U', user2, NULL));

	// untrusting
	ASSERT_EQUAL(memCacheUntrust(memcache, user1, user2), MEMCACHE_SUCCESS);

	// chmods
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1G1, 'G'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block1G1, 42, 'G', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1A1, 'A'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block1A1, 13, 'A', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block2G1, 'G'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block2G1, 42, 'G', user2, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block2A1, 'A'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block2A1, 13, 'A', user2, NULL));

	// frees
	ASSERT_EQUAL(memCacheFree(memcache, user1, block2U1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_EQUAL(memCacheFree(memcache, user1, block2G1), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheFree(memcache, user1, block2A1), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheFree(memcache, user2, block1U1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_EQUAL(memCacheFree(memcache, user2, block1G1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_EQUAL(memCacheFree(memcache, user2, block1A1), MEMCACHE_SUCCESS);

	memCacheDestroy(memcache);

	return true;
}

static bool memCacheAllocateTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	const char * const user1 = "gammaray";
	const char * const user2 = "scorpion";
	const char * const user3 = "amaranth";
	const char * const user4 = "nightwis";

	// add users to system
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, 256), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user2, 266), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user3, 300), MEMCACHE_SUCCESS);

	ASSERT_NULL(memCacheAllocate(NULL, NULL, 0));
	ASSERT_NULL(memCacheAllocate(memcache, NULL, 0));
	ASSERT_NULL(memCacheAllocate(memcache, "user", 0));
	ASSERT_NULL(memCacheAllocate(memcache, user4, 0));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 0));
	ASSERT_NULL(memCacheAllocate(memcache, user1, -1));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 14));
	void *block11 = memCacheAllocate(memcache, user1, 256);
	ASSERT_NOT_NULL(block11);
	ASSERT_TRUE(checkBlock(block11, 256, 'U', user1, NULL));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 1));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block11, 'A'), MEMCACHE_SUCCESS);
	ASSERT_TRUE(checkBlock(block11, 256, 'A', NULL));
	ASSERT_EQUAL(memCacheFree(memcache, user2, block11), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(block11, memCacheGetFirstFreeBlock(memcache));
	void *block21 = memCacheAllocate(memcache, user2, 256);
	ASSERT_EQUAL(block11, block21);
	ASSERT_TRUE(checkBlock(block21, 256, 'U', user2, NULL));
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));
	ASSERT_EQUAL(memCacheFree(memcache, user2, block21), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(block21, memCacheGetFirstFreeBlock(memcache));
	void *block22 = memCacheAllocate(memcache, user2, 256);
	ASSERT_EQUAL(block21, block22);
	ASSERT_TRUE(checkBlock(block22, 256, 'U', user2, NULL));
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));
	void *block31 = memCacheAllocate(memcache, user3, 257);
	ASSERT_NOT_NULL(block31);
	ASSERT_EQUAL(memCacheFree(memcache, user3, block31), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheFreeTest() {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	const char * const user1 = "gammaray";
	const char * const user2 = "scorpion";
	const char * const user3 = "amaranth";
	const char * const user4 = "nightwis";

	// add users to system
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, 256), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user2, 266), MEMCACHE_SUCCESS);
	ASSERT_EQUAL(memCacheAddUser(memcache, user3, 300), MEMCACHE_SUCCESS);

	ASSERT_EQUAL(memCacheFree(memcache, user1, NULL) == MEMCACHE_BLOCK_NOT_ALLOCATED);
	// TODO finish writing
}

static bool memCacheAllocatedBlockForeachTest() {
	// NULL stable
	ASSERT_NULL(memCacheGetFirstAllocatedBlock(NULL));
	ASSERT_NULL(memCacheGetCurrentAllocatedBlock(NULL));
	ASSERT_NULL(memCacheGetNextAllocatedBlock(NULL));

	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);
	// empty test
	ASSERT_NULL(memCacheGetFirstAllocatedBlock(memcache));
	ASSERT_NULL(memCacheGetCurrentAllocatedBlock(memcache));
	ASSERT_NULL(memCacheGetNextAllocatedBlock(memcache));

	// users
	const char * const user = "oomph!!!";

	// add 2 system
	ASSERT_EQUAL(memCacheAddUser(memcache, user, 393354), MEMCACHE_SUCCESS);

	// allocations
	const int SIZE2ALLOCATE_LENGTH = 9;
	const int SIZE2ALLOCATE[SIZE2ALLOCATE_LENGTH] = {1, 1, 1, 23, 23, 23, (1<<16), (1<<16), (1<<16)};
	void * blocks[SIZE2ALLOCATE_LENGTH];
	int visitedTimes[SIZE2ALLOCATE_LENGTH] = {0};

	for (int i = 0; i < SIZE2ALLOCATE_LENGTH; ++i) {
		blocks[i] = memCacheAllocate(memcache, user, SIZE2ALLOCATE[i]);
		ASSERT_NOT_NULL(blocks[i]);
	}

	int iterations = 0;
	MEMCACHE_ALLOCATED_FOREACH(void*, block, memcache) {
		++iterations;
		for (int i = 0; i < SIZE2ALLOCATE_LENGTH; ++i) {
			if (blocks[i] == block) {
				++visitedTimes[i];
			}
		}
	}

	ASSERT_EQUAL(iterations, SIZE2ALLOCATE_LENGTH);
	for (int i = 0; i < SIZE2ALLOCATE_LENGTH; ++i) {
		ASSERT_EQUAL(visitedTimes[i], 1);
	}

	return true;
}

static bool memCacheFreeBlockForeachTest() {
	// TODO write it
}

int main() {
  RUN_TEST(memCacheExampleTest);
  RUN_TEST(memCacheDestroyTest);
  RUN_TEST(memCacheSetBlockModTest);
  RUN_TEST(memCacheUntrustTest);
  RUN_TEST(memCacheAllocateTest);
  RUN_TEST(memCacheAllocatedBlockForeachTest);
  RUN_TEST(memCacheFreeBlockForeachTest);

  return 0;
}

