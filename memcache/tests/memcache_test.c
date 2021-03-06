#include "test_utilities.h"
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include "../memcache.h"

#define ASSERT_TRUE(x) ASSERT_TEST(x);
#define ASSERT_EQUAL(x,y) ASSERT_TEST((x) == (y));
#define ASSERT_EQUAL_STR(x,y) ASSERT_TEST(strcmp((x),(y)) == 0);
#define ASSERT_SUCCESS(x) ASSERT_EQUAL((x), MEMCACHE_SUCCESS);
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

static bool memCacheCreateTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);
	memCacheDestroy(memcache);
	return true;
}

static bool memCacheDestroyTest(void) {
	// is NULL stable
	memCacheDestroy(NULL);

	// use all possible structures to detect leak
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);
	const int ALLOCATED_CACHE_SIZE = (1<<10);
	const int FREE_CACHE_SIZE = 256;
	const int ALL_POSSIBLE_ALLOCATED_BLOCKS_SUM = (1+ALLOCATED_CACHE_SIZE)*ALLOCATED_CACHE_SIZE/2;
	const int ALL_POSSIBLE_FREE_BLOCKS_SUM = (1+FREE_CACHE_SIZE)*FREE_CACHE_SIZE/2;
	// add users
	char * username1 = "LepsGena";
	char * username2 = "Coldplay";
	ASSERT_SUCCESS(memCacheAddUser(memcache, username1, ALL_POSSIBLE_ALLOCATED_BLOCKS_SUM));
	ASSERT_SUCCESS(memCacheAddUser(memcache, username2, ALL_POSSIBLE_FREE_BLOCKS_SUM));

	//allocate memory
	void *user1Memory[ALLOCATED_CACHE_SIZE];
	void *user2Memory[FREE_CACHE_SIZE];
	for (int size = 1; size <= ALLOCATED_CACHE_SIZE; ++size) {
		user1Memory[size-1] = memCacheAllocate(memcache, username1, size);
		ASSERT_NOT_NULL(user1Memory[size-1]);
		ASSERT_TRUE(checkBlock(user1Memory[size-1], size, 'U', username1, NULL));
	}
	for (int size = 1; size <= FREE_CACHE_SIZE; ++size) {
		user2Memory[size-1] = memCacheAllocate(memcache, username2, size);
		ASSERT_NOT_NULL(user2Memory[size-1]);
		memCacheSetBlockMod(memcache, username2, user2Memory[size-1], 'G');
		ASSERT_TRUE(checkBlock(user2Memory[size-1], size, 'G', username2, NULL));
	}

	//user1 and user2 trust each other
	ASSERT_SUCCESS(memCacheTrust(memcache, username1, username2));
	ASSERT_SUCCESS(memCacheTrust(memcache, username2, username1));

	//user1 frees all memory user2 has, while latter is sleeping
	for (int size = 1; size <= FREE_CACHE_SIZE; ++size) {
		ASSERT_SUCCESS(memCacheFree(memcache, username1, user2Memory[size-1]));
	}

	//user2 doesn't trusts to user1 anymore
	ASSERT_SUCCESS(memCacheUntrust(memcache, username2, username1));

	//start some iterations
	ASSERT_NOT_NULL(memCacheGetFirstFreeBlock(memcache));
	ASSERT_NOT_NULL(memCacheGetFirstAllocatedBlock(memcache));

	memCacheDestroy(memcache);

	return true;
}

static bool memCacheAddUserTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	char *user1 = "AxelRudi";
	char *shortUser = "AxelRud";
	char *longUser = "AxelRudiP";
	char *emptyUser = "";

	ASSERT_EQUAL(memCacheAddUser(NULL, user1, 23), MEMCACHE_NULL_ARGUMENT);
	ASSERT_EQUAL(memCacheAddUser(NULL, user1, -1), MEMCACHE_NULL_ARGUMENT);
	ASSERT_EQUAL(memCacheAddUser(memcache, NULL, 23), MEMCACHE_ILLEGAL_USERNAME);
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, 0), MEMCACHE_INVALID_ARGUMENT);
	ASSERT_EQUAL(memCacheAddUser(memcache, user1, -1), MEMCACHE_INVALID_ARGUMENT);
	ASSERT_EQUAL(memCacheAddUser(memcache, shortUser, -1), MEMCACHE_INVALID_ARGUMENT);
	ASSERT_EQUAL(memCacheAddUser(memcache, shortUser, 23), MEMCACHE_ILLEGAL_USERNAME);
	ASSERT_EQUAL(memCacheAddUser(memcache, longUser, 23), MEMCACHE_ILLEGAL_USERNAME);
	ASSERT_EQUAL(memCacheAddUser(memcache, emptyUser, 23), MEMCACHE_ILLEGAL_USERNAME);
	for (int i = 0; i < 8; ++i) {
		for (int c = 0; c < 256; ++c) {
			if (!isalnum(c)) {
				char illegalName[9];
				strcpy(illegalName, "legal123");
				illegalName[i] = c;
				ASSERT_EQUAL(memCacheAddUser(memcache, illegalName, 23), MEMCACHE_ILLEGAL_USERNAME);
			}
		}
	}

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheSetBlockModTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	char * user1 = "gammaray";
	char * user2 = "scorpion";
	char * user3 = "amaranth";
	char * user4 = "nightwis";

	// add users to system
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 100));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 100));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 100));

	// relations
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user2));

	// allocations
	void *block1 = memCacheAllocate(memcache, user1, 10);
	ASSERT_NOT_NULL(block1);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	void *block2 = memCacheAllocate(memcache, user2, 23);
	ASSERT_NOT_NULL(block2);
	ASSERT_TRUE(checkBlock(block2, 23, 'U', user2, NULL));

	// releasing
	ASSERT_SUCCESS(memCacheFree(memcache, user2, block2));


	ASSERT_EQUAL(memCacheSetBlockMod(NULL, NULL, NULL, 'B'), MEMCACHE_NULL_ARGUMENT);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, NULL, NULL, 'B'), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user4, NULL, 'B'), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, NULL, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, (char*)block1+1, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block2, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block2, 'B'), MEMCACHE_BLOCK_NOT_ALLOCATED);
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block1, 'B'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user1, block1, 'B'), MEMCACHE_INVALID_ARGUMENT);
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1, 'U'));
	ASSERT_TRUE(checkBlock(block1, 10, 'U', user1, NULL));

	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1, 'G'));
	ASSERT_TRUE(checkBlock(block1, 10, 'G', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, block1, 'A'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'G', user1, NULL));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1, 'A'));
	ASSERT_TRUE(checkBlock(block1, 10, 'A', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user3, block1, 'U'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_TRUE(checkBlock(block1, 10, 'A', user1, NULL));

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheTrustTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	char *user1 = "scorpion";
	char *user2 = "tarakany";
	char *user3 = "lumen000";
	char *hacker = "rammstei";

	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 4223));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 4323));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 4323));

	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user2));
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user1));
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user3));
	ASSERT_SUCCESS(memCacheUntrust(memcache, user1, user3));
	ASSERT_EQUAL(memCacheTrust(memcache, hacker, user2), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheTrust(memcache, user1, hacker), MEMCACHE_USER_NOT_FOUND);

	void *blockU = memCacheAllocate(memcache, user1, 42);
	ASSERT_TRUE(checkBlock(blockU, 42, 'U', user1, NULL));

	void *blockG = memCacheAllocate(memcache, user1, 23);
	ASSERT_TRUE(checkBlock(blockG, 23, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, blockG, 'G'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, blockG, 'G'));
	ASSERT_TRUE(checkBlock(blockG, 23, 'G', user1, NULL));

	void *blockA = memCacheAllocate(memcache, user1, 23);
	ASSERT_TRUE(checkBlock(blockA, 23, 'U', user1, NULL));
	ASSERT_EQUAL(memCacheSetBlockMod(memcache, user2, blockA, 'A'), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, blockA, 'A'));
	ASSERT_TRUE(checkBlock(blockA, 23, 'A', user1, NULL));

	ASSERT_EQUAL(memCacheFree(memcache, hacker, blockU), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheFree(memcache, hacker, blockG), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheFree(memcache, hacker, blockA), MEMCACHE_USER_NOT_FOUND);

	ASSERT_EQUAL(memCacheFree(memcache, user3, blockU), MEMCACHE_PERMISSION_DENIED);
	ASSERT_EQUAL(memCacheFree(memcache, user3, blockG), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheFree(memcache, user3, blockA));


	ASSERT_EQUAL(memCacheFree(memcache, user2, blockU), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheFree(memcache, user2, blockG));
	ASSERT_EQUAL(memCacheFree(memcache, user2, blockA), MEMCACHE_BLOCK_NOT_ALLOCATED);

	ASSERT_SUCCESS(memCacheFree(memcache, user1, blockU));
	ASSERT_EQUAL(memCacheFree(memcache, hacker, blockG), MEMCACHE_USER_NOT_FOUND);
	ASSERT_EQUAL(memCacheFree(memcache, hacker, blockA), MEMCACHE_USER_NOT_FOUND);

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheUntrustTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	char * user1 = "gammaray";
	char * user2 = "scorpion";
	char * user3 = "amaranth";
	char * user4 = "radiohea";

	// add users to system
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 100));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 100));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 100));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user4, 100));

	// trusting
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user2));
	ASSERT_SUCCESS(memCacheTrust(memcache, user2, user1));
	ASSERT_SUCCESS(memCacheTrust(memcache, user4, user4));
	ASSERT_SUCCESS(memCacheTrust(memcache, user4, user4));

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
	void *block4U1 = memCacheAllocate(memcache, user4, 23);


	// untrusting
	ASSERT_SUCCESS(memCacheUntrust(memcache, user1, user2));
	ASSERT_SUCCESS(memCacheUntrust(memcache, user4, user4));
	ASSERT_SUCCESS(memCacheUntrust(memcache, user4, user4));

	// chmods
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1G1, 'G'));
	ASSERT_TRUE(checkBlock(block1G1, 42, 'G', user1, NULL));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1A1, 'A'));
	ASSERT_TRUE(checkBlock(block1A1, 13, 'A', user1, NULL));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user2, block2G1, 'G'));
	ASSERT_TRUE(checkBlock(block2G1, 42, 'G', user2, NULL));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user2, block2A1, 'A'));
	ASSERT_TRUE(checkBlock(block2A1, 13, 'A', user2, NULL));

	// frees
	ASSERT_EQUAL(memCacheFree(memcache, user1, block2U1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheFree(memcache, user1, block2G1));
	ASSERT_SUCCESS(memCacheFree(memcache, user1, block2A1));
	ASSERT_EQUAL(memCacheFree(memcache, user2, block1U1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_EQUAL(memCacheFree(memcache, user2, block1G1), MEMCACHE_PERMISSION_DENIED);
	ASSERT_SUCCESS(memCacheFree(memcache, user2, block1A1));
	ASSERT_SUCCESS(memCacheFree(memcache, user4, block4U1));

	memCacheDestroy(memcache);

	return true;
}

static bool memCacheAllocateTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	char * user1 = "gammaray";
	char * user2 = "scorpion";
	char * user3 = "amaranth";
	char * user4 = "nightwis";

	// add users to system
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 256));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 266));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 300));

	ASSERT_NULL(memCacheAllocate(NULL, NULL, 0));
	ASSERT_NULL(memCacheAllocate(memcache, NULL, 0));
	ASSERT_NULL(memCacheAllocate(memcache, "user", 0));
	ASSERT_NULL(memCacheAllocate(memcache, user4, 0));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 0));
	ASSERT_NULL(memCacheAllocate(memcache, user1, -1));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 257));
	void *block11 = memCacheAllocate(memcache, user1, 256);
	ASSERT_NOT_NULL(block11);
	ASSERT_TRUE(checkBlock(block11, 256, 'U', user1, NULL));
	ASSERT_NULL(memCacheAllocate(memcache, user1, 1));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block11, 'A'));
	ASSERT_TRUE(checkBlock(block11, 256, 'A', user1, NULL));
	ASSERT_SUCCESS(memCacheFree(memcache, user2, block11));
	ASSERT_EQUAL(block11, memCacheGetFirstFreeBlock(memcache));
	void *block21 = memCacheAllocate(memcache, user2, 256);
	ASSERT_EQUAL(block11, block21);
	ASSERT_TRUE(checkBlock(block21, 256, 'U', user2, NULL));
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));
	ASSERT_SUCCESS(memCacheFree(memcache, user2, block21));
	ASSERT_EQUAL(block21, memCacheGetFirstFreeBlock(memcache));
	void *block22 = memCacheAllocate(memcache, user2, 256);
	ASSERT_EQUAL(block21, block22);
	ASSERT_TRUE(checkBlock(block22, 256, 'U', user2, NULL));
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));
	void *block31 = memCacheAllocate(memcache, user3, 257);
	ASSERT_NOT_NULL(block31);
	ASSERT_SUCCESS(memCacheFree(memcache, user3, block31));
	ASSERT_EQUAL(NULL, memCacheGetFirstFreeBlock(memcache));

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheFreeTest() {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	//users
	char * user1 = "gammaray";
	char * user2 = "scorpion";
	char * user3 = "amaranth";
	char * user4 = "nightwis";

	// add users to system
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 3*(1+257)*257/2));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 266));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 300));

	// relations
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user2));
	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user3));
	ASSERT_SUCCESS(memCacheUntrust(memcache, user1, user3));

	ASSERT_EQUAL(memCacheFree(memcache, user1, NULL), MEMCACHE_BLOCK_NOT_ALLOCATED);

	for(int size = 4; size <= 257; ++size) {
		void *block1 = NULL, *block2 = NULL, *block3 = NULL;
		ASSERT_NOT_NULL(block1 = memCacheAllocate(memcache, user1, size));
		ASSERT_NOT_NULL(block2 = memCacheAllocate(memcache, user1, size));
		ASSERT_NOT_NULL(block3 = memCacheAllocate(memcache, user1, size));
		ASSERT_TRUE(checkBlock(block1, size, 'U', user1, NULL));
		ASSERT_TRUE(checkBlock(block2, size, 'U', user1, NULL));
		ASSERT_TRUE(checkBlock(block2, size, 'U', user1, NULL));

		char *fake_block = (char*)calloc(sizeof(int)+1+1+8+2+size+1, sizeof(char));
		ASSERT_NOT_NULL(fake_block);
		*(int*)fake_block = size;
		*(fake_block+sizeof(int)+1) = 'U';
		strcpy(fake_block+sizeof(int)+1+1, user1);
		ASSERT_TRUE(checkBlock(fake_block+sizeof(int)+1+1+8+2, size, 'U', user1, NULL));
		ASSERT_EQUAL(memCacheFree(memcache, user4, fake_block), MEMCACHE_USER_NOT_FOUND);
		ASSERT_EQUAL(memCacheFree(memcache, user2, fake_block), MEMCACHE_BLOCK_NOT_ALLOCATED);
		ASSERT_EQUAL(memCacheFree(memcache, user1, fake_block+sizeof(int)+1+1+8+2), MEMCACHE_BLOCK_NOT_ALLOCATED);

		free(fake_block);

		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block2, 'G'));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block3, 'A'));
		ASSERT_TRUE(checkBlock(block1, size, 'U', user1, NULL));
		ASSERT_TRUE(checkBlock(block2, size, 'G', user1, NULL));
		ASSERT_TRUE(checkBlock(block3, size, 'A', user1, NULL));
		sprintf(block1, "0%d", size);
		sprintf(block2, "1%d", size);
		sprintf(block3, "2%d", size);
		ASSERT_EQUAL(memCacheFree(memcache, user4, block1), MEMCACHE_USER_NOT_FOUND);
		ASSERT_EQUAL(memCacheFree(memcache, user3, block1), MEMCACHE_PERMISSION_DENIED);
		ASSERT_EQUAL(memCacheFree(memcache, user2, block1), MEMCACHE_PERMISSION_DENIED);
		ASSERT_SUCCESS(memCacheFree(memcache, user1, block1));
		ASSERT_EQUAL(memCacheFree(memcache, user1, block1), MEMCACHE_BLOCK_NOT_ALLOCATED);
		ASSERT_EQUAL(memCacheFree(memcache, user4, block2), MEMCACHE_USER_NOT_FOUND);
		ASSERT_EQUAL(memCacheFree(memcache, user3, block2), MEMCACHE_PERMISSION_DENIED);
		ASSERT_SUCCESS(memCacheFree(memcache, user2, block2));
		ASSERT_EQUAL(memCacheFree(memcache, user2, block2), MEMCACHE_BLOCK_NOT_ALLOCATED);
		ASSERT_EQUAL(memCacheFree(memcache, user4, block3), MEMCACHE_USER_NOT_FOUND);
		ASSERT_SUCCESS(memCacheFree(memcache, user3, block3));
		ASSERT_EQUAL(memCacheFree(memcache, user3, block3), MEMCACHE_BLOCK_NOT_ALLOCATED);
	}

	int cnt[3][256] = {{0}};
	MEMCACHE_FREE_FOREACH(block, memcache) {
		int modeIndex = *(char*)block - '0';
		ASSERT_TRUE(0 <= modeIndex && modeIndex < 3);
		int size = (int)strtoimax((char*)block+1, NULL, 10);
		ASSERT_TRUE(1 <= size && size <= 256);
		char mod = modeIndex == 0 ? 'U' : modeIndex == 1 ? 'G' : 'A';
		char data[10];
		sprintf(data, "%d%d", modeIndex, size);
		ASSERT_TRUE(checkBlock(block, size, mod, user1, data));
		++cnt[modeIndex][size-1];
	}

	for (int modeIndex = 0; modeIndex < 3; ++modeIndex) {
		for (int size = 4; size <= 256; ++size) {
			ASSERT_TRUE(cnt[modeIndex][size-1] == 1);
		}
	}

	memCacheDestroy(memcache);
	return true;
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
	char * user = "oomph123";

	// add to system
	const int MODULO = 1024;
	const int MAX_SIZE = 1024 * 2;
	ASSERT_SUCCESS(memCacheAddUser(memcache, user, 6*(1+MAX_SIZE)*MAX_SIZE/2));

	// allocations
	void *allocatedBlocks[3][MAX_SIZE], *freeBlocks[3][MAX_SIZE];
	int visited[3][MAX_SIZE];
	for (int size = 1; size <= MAX_SIZE; ++size) {
		for (int i = 0; i < 3; ++i) {
			visited[i][size-1] = 0;
		}
		ASSERT_NOT_NULL(allocatedBlocks[0][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(allocatedBlocks[0][size-1], size, 'U', user, NULL));
		ASSERT_NOT_NULL(freeBlocks[0][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(freeBlocks[0][size-1], size, 'U', user, NULL));

		ASSERT_NOT_NULL(allocatedBlocks[1][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(allocatedBlocks[1][size-1], size, 'U', user, NULL));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, allocatedBlocks[1][size-1], 'G'));
		ASSERT_TRUE(checkBlock(allocatedBlocks[1][size-1], size, 'G', user, NULL));
		ASSERT_NOT_NULL(freeBlocks[1][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(freeBlocks[1][size-1], size, 'U', user, NULL));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, freeBlocks[1][size-1], 'G'));
		ASSERT_TRUE(checkBlock(freeBlocks[1][size-1], size, 'G', user, NULL));

		ASSERT_NOT_NULL(allocatedBlocks[2][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(allocatedBlocks[2][size-1], size, 'U', user, NULL));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, allocatedBlocks[2][size-1], 'A'));
		ASSERT_TRUE(checkBlock(allocatedBlocks[2][size-1], size, 'A', user, NULL));
		ASSERT_NOT_NULL(freeBlocks[2][size-1] = memCacheAllocate(memcache, user, size));
		ASSERT_TRUE(checkBlock(freeBlocks[2][size-1], size, 'U', user, NULL));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, freeBlocks[2][size-1], 'A'));
		ASSERT_TRUE(checkBlock(freeBlocks[2][size-1], size, 'A', user, NULL));
	}


	for (int size = 1; size <= MAX_SIZE; ++size) {
		for (int i = 0; i < 3; ++i) {
			ASSERT_SUCCESS(memCacheFree(memcache, user, freeBlocks[i][size-1]));
		}
	}

	int currentReminder = 0;
	MEMCACHE_ALLOCATED_FOREACH(block, memcache) {
		ASSERT_TRUE((uintptr_t)block % MODULO >= currentReminder);
		currentReminder = (uintptr_t)block % MODULO;
		bool found = false;
		for (int mod = 0; mod < 3 && !found; ++mod) {
			for (int size = 1; size <= MAX_SIZE && !found; ++size) {
				if (allocatedBlocks[mod][size-1] == block) {
					++visited[mod][size-1];
					found = true;
				}
			}
		}
		ASSERT_TRUE(found);
	}
	for (int size = 1; size <= MAX_SIZE; ++size) {
		for (int i = 0; i < 3; ++i) {
			ASSERT_EQUAL(visited[i][size-1], 1);
		}
	}

	ASSERT_NULL(memCacheGetCurrentAllocatedBlock(memcache));
	ASSERT_NULL(memCacheGetNextAllocatedBlock(memcache));


	memCacheDestroy(memcache);

	return true;
}

static bool memCacheFreeTest2(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	const int size = 50;

	char *user = "ironmaid";

	char *content1 = "block1 of ironmaid";
	char *content2 = "block2 of ironmaid";

	ASSERT_SUCCESS(memCacheAddUser(memcache, user, 2*size));

	void *block1 = memCacheAllocate(memcache, user, size);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user, NULL));
	void *block2 = memCacheAllocate(memcache, user, size);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user, NULL));

	sprintf(block1, content1);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user, content1));
	sprintf(block2, content2);
	ASSERT_TRUE(checkBlock(block2, size, 'U', user, content2));

	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, block1, 'A'));
	ASSERT_TRUE(checkBlock(block1, size, 'A', user, content1));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user, block2, 'G'));
	ASSERT_TRUE(checkBlock(block2, size, 'G', user, content2));

	ASSERT_SUCCESS(memCacheFree(memcache, user, block1));
	ASSERT_SUCCESS(memCacheFree(memcache, user, block2));

	ASSERT_TRUE(checkBlock(memCacheGetFirstFreeBlock(memcache), size, 'A', user, content1) ||
			checkBlock(memCacheGetFirstFreeBlock(memcache), size, 'G', user, content2));

	ASSERT_TRUE(memCacheGetFirstFreeBlock(memcache) == block1 ||
			memCacheGetFirstFreeBlock(memcache) == block2);

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheFreeTest3(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	const int size = 50;

	char *user1 = "ironmaid";
	char *user2 = "edguy423";
	char *user3 = "paradise";

	char *content1 = "block1 of ironmaid";
	char *content2 = "block2 of ironmaid";

	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 2*size));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 2*size));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, 2*size));

	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user3));

	void *block1 = memCacheAllocate(memcache, user1, size);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user1, NULL));
	void *block2 = memCacheAllocate(memcache, user1, size);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user1, NULL));

	sprintf(block1, content1);
	ASSERT_TRUE(checkBlock(block1, size, 'U', user1, content1));
	sprintf(block2, content2);
	ASSERT_TRUE(checkBlock(block2, size, 'U', user1, content2));

	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block1, 'A'));
	ASSERT_TRUE(checkBlock(block1, size, 'A', user1, content1));
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block2, 'G'));
	ASSERT_TRUE(checkBlock(block2, size, 'G', user1, content2));

	ASSERT_SUCCESS(memCacheFree(memcache, user2, block1));
	ASSERT_SUCCESS(memCacheFree(memcache, user3, block2));

	ASSERT_TRUE(checkBlock(memCacheGetFirstFreeBlock(memcache), size, 'A', user1, content1) ||
			checkBlock(memCacheGetFirstFreeBlock(memcache), size, 'G', user1, content2));

	ASSERT_TRUE(memCacheGetFirstFreeBlock(memcache) == block1 ||
			memCacheGetFirstFreeBlock(memcache) == block2);

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheFreeBlockForeachTest(void) {
	// NULL stable
	ASSERT_NULL(memCacheGetCurrentFreeBlock(NULL));
	ASSERT_NULL(memCacheGetFirstFreeBlock(NULL));
	ASSERT_NULL(memCacheGetNextFreeBlock(NULL));

	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	// empty stable
	ASSERT_NULL(memCacheGetCurrentFreeBlock(NULL));
	ASSERT_NULL(memCacheGetFirstFreeBlock(NULL));
	ASSERT_NULL(memCacheGetNextFreeBlock(NULL));

	//users
	char *user1 = "accept12";
	char *user2 = "offsprin";
	char *user3 = "nightwis";
	char *user4 = "johndoe1";

	// add to system
	const int MAX_SIZE = 258;
	const int MEMORY_LIMIT = (1+MAX_SIZE) * MAX_SIZE / 2;
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, MEMORY_LIMIT));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, MEMORY_LIMIT));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user3, MEMORY_LIMIT));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user4, MEMORY_LIMIT));

	// trusts
	ASSERT_SUCCESS(memCacheTrust(memcache, user2, user4));

	//Empty cache
	ASSERT_NULL(memCacheGetCurrentFreeBlock(memcache));
	ASSERT_NULL(memCacheGetFirstFreeBlock(memcache));
	ASSERT_NULL(memCacheGetNextFreeBlock(memcache));

	// allocate blocks
	void *user1Blocks[258] = {NULL}, *user2Blocks[258] = {NULL}, *user3Blocks[258] = {NULL}, *user4Blocks[258] = {NULL};
	for (int size = 1; size <= 258; ++size) {
		ASSERT_NOT_NULL(user1Blocks[size-1] = memCacheAllocate(memcache, user1, size));
		ASSERT_TRUE(checkBlock(user1Blocks[size-1], size, 'U', user1, NULL));
		ASSERT_NOT_NULL(user2Blocks[size-1] = memCacheAllocate(memcache, user2, size));
		ASSERT_TRUE(checkBlock(user2Blocks[size-1], size, 'U', user2, NULL));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user2, user2Blocks[size-1], 'G'));
		ASSERT_TRUE(checkBlock(user2Blocks[size-1], size, 'G', user2, NULL));
		ASSERT_NOT_NULL(user3Blocks[size-1] = memCacheAllocate(memcache, user3, size));
		ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user3, user3Blocks[size-1], 'A'));
		ASSERT_TRUE(checkBlock(user3Blocks[size-1], size, 'A', user3, NULL));
		ASSERT_NOT_NULL(user4Blocks[size-1] = memCacheAllocate(memcache, user4, size));
	}

	// deallocate blocks
	void *freedBlocks[3*256];
	int visited[3*256] = {0};
	for (int size = 1; size <= 256; ++size) {
		ASSERT_SUCCESS(memCacheFree(memcache, user1, user1Blocks[size-1]));
		freedBlocks[3*(size-1)] = user1Blocks[size-1];
		ASSERT_SUCCESS(memCacheFree(memcache, user4, user2Blocks[size-1]));
		freedBlocks[3*(size-1)+1] = user2Blocks[size-1];
		ASSERT_EQUAL(memCacheFree(memcache, user2, user2Blocks[size-1]), MEMCACHE_BLOCK_NOT_ALLOCATED);
		ASSERT_SUCCESS(memCacheFree(memcache, user4, user3Blocks[size-1]));
		freedBlocks[3*(size-1)+2] = user3Blocks[size-1];
	}
	ASSERT_SUCCESS(memCacheFree(memcache, user1, user1Blocks[257-1]));
	ASSERT_SUCCESS(memCacheFree(memcache, user4, user2Blocks[257-1]));
	ASSERT_SUCCESS(memCacheFree(memcache, user4, user3Blocks[257-1]));

	int currentSize = 1, blocksOfCurrentSize = 0;
	MEMCACHE_FREE_FOREACH(block, memcache) {
		for (int i = 0; i < 3 * 256; ++i) {
			if (freedBlocks[i] == block) {
				++visited[i];
				int size = i / 3 + 1;
				ASSERT_EQUAL(size, currentSize);
			}
		}
		if (++blocksOfCurrentSize == 3) {
			++currentSize;
			blocksOfCurrentSize = 0;
		}
	}

	ASSERT_EQUAL(currentSize, 257);
	ASSERT_EQUAL(blocksOfCurrentSize, 0);

	for (int i = 0; i < 3 * 256; ++i) {
		ASSERT_EQUAL(visited[i], 1);
	}

	memCacheDestroy(memcache);
	return true;
}

static bool memCacheResetTest(void) {
	MemCache memcache = memCacheCreate();
	ASSERT_NOT_NULL(memcache);

	char *user1 = "skillet1";
	char *user2 = "forsaken";

	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 42));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 42));

	ASSERT_SUCCESS(memCacheTrust(memcache, user1, user2));

	void *block1 = memCacheAllocate(memcache, user1, 17);
	ASSERT_TRUE(checkBlock(block1, 17, 'U', user1, NULL));
	void *block2 = memCacheAllocate(memcache, user1, 17);
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block2, 'G'));
	ASSERT_TRUE(checkBlock(block2, 17, 'G', user1, NULL));

	ASSERT_EQUAL(memCacheReset(NULL), MEMCACHE_NULL_ARGUMENT);
	ASSERT_SUCCESS(memCacheReset(memcache));

	ASSERT_NULL(memCacheAllocate(memcache, user1, 1));
	ASSERT_EQUAL(memCacheFree(memcache, user1, block1), MEMCACHE_USER_NOT_FOUND);
	ASSERT_SUCCESS(memCacheAddUser(memcache, user1, 42));
	ASSERT_SUCCESS(memCacheAddUser(memcache, user2, 42));
	ASSERT_EQUAL(memCacheFree(memcache, user1, block1), MEMCACHE_BLOCK_NOT_ALLOCATED);
	block2 = memCacheAllocate(memcache, user1, 42);
	ASSERT_SUCCESS(memCacheSetBlockMod(memcache, user1, block2, 'G'));
	ASSERT_TRUE(checkBlock(block2, 42, 'G', user1, NULL));
	ASSERT_EQUAL(memCacheFree(memcache, user2, block2), MEMCACHE_PERMISSION_DENIED);

	memCacheDestroy(memcache);
	return true;
}

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	RUN_TEST(memCacheExampleTest);
	RUN_TEST(memCacheCreateTest);
	RUN_TEST(memCacheDestroyTest);
	RUN_TEST(memCacheAddUserTest);
	RUN_TEST(memCacheSetBlockModTest);
	RUN_TEST(memCacheTrustTest);
	RUN_TEST(memCacheUntrustTest);
	RUN_TEST(memCacheAllocateTest);
	RUN_TEST(memCacheFreeTest);
	RUN_TEST(memCacheFreeTest2);
	RUN_TEST(memCacheFreeTest3);
	RUN_TEST(memCacheAllocatedBlockForeachTest);
	RUN_TEST(memCacheFreeBlockForeachTest);
	RUN_TEST(memCacheResetTest);

	return 0;
}


