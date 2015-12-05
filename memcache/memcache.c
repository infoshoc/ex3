/*
 * memcache.c
 *
 *  Created on: Nov 30, 2015
 *      Author: Infoshoc_2
 */

#include <stdlib.h>
#include <assert.h>
#include "memcache.h"
#include "cache.h"
#include "graph.h"
#include "map.h"
#include "set.h"

#define MEMCACHE_AVAILIBLE_BLOCK_MAX_SIZE (265)
#define MEMCACHE_ALLOCATED_BLOCK_MODULO (65536)
#define MEMCACHE_USER_NAME_LENGTH (8)

typedef struct MemCache_t {
	Cache freeBlocks;
	Cache allocatedBlocks;
	Graph userRelations;
	Map userMemoryLimit;

	/* used for iteration */
	Set allAllocatedBlocks;
	Set allFreeBlocks;
} MemCache_t;

typedef enum MemCacheBlockMode {
	USER = 'U', GROUP = 'G', ALL = 'A'
} MemCacheBlockMode;

typedef const MemCacheBlockMode ConstMemCacheBlockMode;
typedef void *MemCacheBlock;
typedef const void* const ConstMemCacheBlock;
typedef char *MemCacheUser;
typedef const char* const ConstMemCacheUser;
typedef int *MemCacheLimit;
typedef const int * const ConstMemCacheLimit;

#define MEMCACHE_ALLOCATE(type, var, error) \
	do {\
		if (!(var = (type*)malloc(sizeof(type)))) { \
			return error;\
		}\
	} while(false);

static void memcacheBlockFree(MemCacheBlock block) {
	// TODO
}
static MemCacheBlock memcacheBlockCopy(MemCacheBlock block) {
	// TODO
}
static int memcacheBlocksCompare(MemCacheBlock a, MemCacheBlock b) {
	return (char*)a - (char*)b;
}
static int memcacheBlockGetSize(MemCacheBlock block) {
	//TODO
}
static MemCacheBlockMode memcacheBlockGetMode(MemCacheBlock block) {
	//TODO
}
/** Function returns owner of block (NOT COPY) */
static ConstMemCacheUser memcacheBlockGetOwner(MemCacheBlock block) {
	//TODO
}
static int memcacheAvailibleBlockComputeKey(MemCacheBlock block) {
	// TODO
}
static int memcacheAllocatedBlockComputeKey(MemCacheBlock block) {
	return memcacheBlockGetSize(block)-1;
}
static MemCacheUser memcacheUserCopy(ConstMemCacheUser user) {
	// TODO
}
static int memcacheUsersCompare(ConstMemCacheUser a, ConstMemCacheUser b) {
	// TODO
}
static void memcacheUserFree(MemCacheUser user) {
	//TODO
}
static MemCacheLimit memcacheLimitCopy(ConstMemCacheLimit limit) {
	//TODO
}
static void memcacheLimitFree(MemCacheLimit limit) {
	//TODO
}
static void memcacheDoNothing(MemCacheBlock){}
static bool memcacheIsUserNameLegal(ConstMemCacheUser user) {
	// TODO
}
static bool memcacheIsUserExists(MemCache memcache, ConstMemCacheUser user) {
	assert(memcache != NULL);
	if (!memcacheIsUserNameLegal(user)) {
		return false;
	}
	assert(mapContains(memcache->userMemoryLimit, user) ==
			graphIsVertexExists(memcache->userRelations, user));

	return mapContains(memcache->userMemoryLimit, user);
}

MemCache memCacheCreate() {
	MemCache memcache;
	MEMCACHE_ALLOCATE(MemCache_t, memcache, NULL);

	memcache->freeBlocks = cacheCreate(
			MEMCACHE_AVAILIBLE_BLOCK_MAX_SIZE,
			memcacheBlockFree,
			memcacheBlockCopy,
			memcacheBlocksCompare,
			memcacheAvailibleBlockComputeKey);

	memcache->allocatedBlocks = cacheCreate(
			MEMCACHE_AVAILIBLE_BLOCK_MAX_SIZE,
			memcacheBlockFree,
			memcacheBlockCopy,
			memcacheBlocksCompare,
			memcacheAllocatedBlockComputeKey);

	memcache->userRelations = graphCreate(
			memcacheUserCopy,
			memcacheUsersCompare,
			memcacheUserFree);

	memcache->userMemoryLimit = mapCreate(
			memcacheLimitCopy,
			memcacheUserCopy,
			memcacheLimitFree,
			memcacheUserFree,
			memcacheUsersCompare);

	memcache->allAllocatedBlocks = setCreate(
			memcacheBlockCopy,
			// it is helpful set, we should not deallocate blocks while clear
			memcacheDoNothing,
			memcacheBlocksCompare
			);

	memcache->allFreeBlocks = setCreate(
			memcacheBlockCopy,
			// it is helpful set, we should not deallocate blocks while clear
			memcacheDoNothing,
			memcacheBlocksCompare
			);

	if (!memcache->freeBlocks ||
			!memcache->allocatedBlocks ||
			!memcache->userRelations ||
			!memcache->allAllocatedBlocks ||
			!memcache->allFreeBlocks) {
		memCacheDestroy(memcache);
		return NULL;
	}

	return memcache;
}

MemCachResult memCacheAddUser(MemCache memcache, const char* const username, int memory_limit) {
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}
	if (memory_limit <= 0) {
		return MEMCACHE_INVALID_ARGUMENT;
	}
	if (!memcacheIsUserNameLegal(username)) {
		return MEMCACHE_ILLEGAL_USERNAME;
	}
	if (memcacheIsUserExists(memcache, username)) {
		return MEMCACHE_USERNAME_ALREADY_USED;
	}

	GraphResult graphAddVertexResult = graphAddVertex(memcache->userRelations, username);
	if (graphAddVertexResult == GRAPH_OUT_OF_MEMORY) {
		return MEMCACHE_OUT_OF_MEMORY;
	}
	assert(graphAddVertexResult == GRAPH_SUCCESS);

	MapResult mapPutResult = mapPut(memcache->userMemoryLimit, username, &memory_limit);
	if (mapPutResult == MAP_OUT_OF_MEMORY) {
		// consistency
		GraphResult graphRemoveVertexResult = graphRemoveVertex(memcache->userRelations, username);
		assert(graphRemoveVertexResult == GRAPH_SUCCESS);
		return MEMCACHE_OUT_OF_MEMORY;
	}
	assert(mapPutResult == MAP_SUCCESS);

	return MEMCACHE_SUCCESS;
}

MemCachResult memCacheTrust(
		MemCache memcache,
		const char* const username1,
		const char* const username2) {
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}

	if (!memcacheIsUserExists(memcache, username1) ||
			!memcacheIsUserExists(memcache, username2)) {
		return MEMCACHE_USER_NOT_FOUND;
	}

	GraphResult graphAddEdgeResult = graphAddDirectedEdge(
			memcache->userRelations,
			username1,
			username2);

	if (graphAddEdgeResult == GRAPH_OUT_OF_MEMORY) {
		return MEMCACHE_OUT_OF_MEMORY;
	}

	assert(graphAddEdgeResult == GRAPH_SUCCESS);

	return MEMCACHE_SUCCESS;
}

MemCachResult memCacheFree(MemCache memcache, const char* const username, void* ptr) {
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}

	if (!memcacheIsUserExists(memcache, username)) {
		return MEMCACHE_USER_NOT_FOUND;
	}

	if (!cacheIsIn(memcache->allocatedBlocks, ptr)) {
		return MEMCACHE_BLOCK_NOT_ALLOCATED;
	}
	ConstMemCacheBlockMode mode = memcacheBlockGetMode(ptr);
	ConstMemCacheUser owner = memcacheBlockGetOwner(ptr);
	switch (mode) {
	case USER:
		if (0 != memcacheUsersCompare(username, owner)) {
			return MEMCACHE_PERMISSION_DENIED;
		}
		break;
	case GROUP:
		if (0 != memcacheUsersCompare(username, owner) &&
				!graphIsDirectedEdge(memcache->userRelations, owner, username)) {
			return MEMCACHE_PERMISSION_DENIED;
		}
		break;
	}

	int blockSize = memcacheBlockGetSize(ptr);
	memcacheUserIncreaseMemoryLimit(owner, blockSize);
	if (blockSize > MEMCACHE_AVAILIBLE_BLOCK_MAX_SIZE) {
		memcacheBlockFree(ptr);
	} else {
		assert(memcacheBlockGetSize(ptr) <= MEMCACHE_AVAILIBLE_BLOCK_MAX_SIZE);
		CacheResult cachePushResult = cachePush(memcache->freeBlocks, ptr);
		if (cachePushResult == CACHE_OUT_OF_MEMORY) {
			return MEMCACHE_OUT_OF_MEMORY;
		}
		assert(CACHE_SUCCESS == cachePushResult);
	}

	cacheExtractElementByKey(memcache->allocatedBlocks, ptr);

	return MEMCACHE_SUCCESS;
}


void* memCacheGetNextAllocatedBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return setGetNext(memcache->allAllocatedBlocks);
}

void* memCacheGetNextFreeBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return setGetNext(memcache->allFreeBlocks);
}

MemCachResult memCacheReset(MemCache memcache) {
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}
	GraphResult graphClearResult = graphClear(memcache->userRelations);
	MapResult mapClearResult = mapClear(memcache->userMemoryLimit);
	CacheResult allocatedCacheClearResult = cacheClear(memcache->allocatedBlocks);
	CacheResult freeCacheClearResult = cacheClear(memcache->freeBlocks);
	SetResult allocatedSetClearResult = setClear(memcache->allAllocatedBlocks);
	SetResult freeSetClearResult = setClear(memcache->allFreeBlocks);
	assert(graphClearResult == GRAPH_SUCCESS &&
			mapClearResult == MAP_SUCCESS &&
			allocatedCacheClearResult == CACHE_SUCCESS &&
			freeCacheClearResult == CACHE_SUCCESS &&
			allocatedSetClearResult == SET_SUCCESS &&
			freeSetClearResult == SET_SUCCESS);

	return MEMCACHE_SUCCESS;
}
