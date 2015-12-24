/*
 * memcache.c
 *
 *  Created on: Nov 30, 2015
 *      Author: Infoshoc_2
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "memcache.h"
#include "cache.h"
#include "graph.h"
#include "map.h"
#include "set.h"

#define MEMCACHE_FREE_BLOCK_MAX_SIZE (256)
#define MEMCACHE_ALLOCATED_BLOCK_MODULO (1024)
#define MEMCACHE_USER_NAME_LENGTH (8)

typedef struct MemCache_t {
	Cache freeBlocks;
	Cache allocatedBlocks;
	Graph userRelations;
	Map userMemoryLimit;
} MemCache_t;

typedef enum MemCacheBlockMode {
	USER = 'U', GROUP = 'G', ALL = 'A'
} MemCacheBlockMode;
typedef const MemCacheBlockMode ConstMemCacheBlockMode;

typedef void *MemCacheBlock;
typedef const void* const ConstMemCacheBlock;

typedef char *MemCacheUser;
typedef const char *ConstMemCacheUser;
typedef int *MemCacheLimit;
typedef const int * const ConstMemCacheLimit;

#define MEMCACHE_ALLOCATE(type, var, error) \
	do {\
		if (!(var = (type*)malloc(sizeof(type)))) { \
			return error;\
		}\
	} while(false)

static void memcacheDoNothing(MemCacheBlock block){}

static MemCacheBlock memcacheBlockCopy(MemCacheBlock block) {
	return block;
}
static int memcacheBlocksCompare(MemCacheBlock block1, MemCacheBlock block2) {
	// in order to escape overflow in difference
	return block1 < block2 ? -1 : block1 == block2 ? 0 : 1;
}
static void memcacheFreeBlock(MemCacheBlock block) {
	free((char*)block-2-MEMCACHE_USER_NAME_LENGTH-1-1-sizeof(int));
}
static inline int memcacheBlockGetSize(MemCacheBlock block) {
	return *(int*)((char*)block-2-MEMCACHE_USER_NAME_LENGTH-1-1-sizeof(int));
}
static inline char* memcacheBlockGetModePointer(MemCacheBlock block) {
	return (char*)block-2-MEMCACHE_USER_NAME_LENGTH-1;
}
static inline MemCacheBlockMode memcacheBlockGetMode(MemCacheBlock block) {
	return *memcacheBlockGetModePointer(block);
}
/** Function returns owner of block (NOT COPY) */
static MemCacheUser memcacheBlockGetOwner(MemCacheBlock block) {
	return (char*)block-2-MEMCACHE_USER_NAME_LENGTH;
}
static int memcacheAllocatedBlockComputeKey(MemCacheBlock block) {
	return (uintptr_t)block % MEMCACHE_ALLOCATED_BLOCK_MODULO;
}
static int memcacheBlockSizeToCacheIndex(int size) {
	return size-1;
}
static int memcacheFreeBlockComputeKey(MemCacheBlock block) {
	return memcacheBlockSizeToCacheIndex(memcacheBlockGetSize(block));
}
static bool memcacheIsUserNameLegal(ConstMemCacheUser user) {
	if (user == NULL) {
		return false;
	}
	if (user[MEMCACHE_USER_NAME_LENGTH] != '\0') {
		return false;
	}
	if (strlen(user) != MEMCACHE_USER_NAME_LENGTH) {
		return false;
	}
	for (int i = 0; i < MEMCACHE_USER_NAME_LENGTH; ++i) {
		if (!isalnum(user[i])) {
			return false;
		}
	}
	return true;
}
static MemCacheUser memcacheUserCopy(ConstMemCacheUser user) {
	assert(memcacheIsUserNameLegal(user));
	MemCacheUser copy = malloc(strlen(user)+1);
	if (copy == NULL) {
		return NULL;
	}

	strcpy(copy, user);
	return copy;

}
static int memcacheUsersCompare(ConstMemCacheUser user1, ConstMemCacheUser user2) {
	assert(memcacheIsUserNameLegal(user1));
	assert(memcacheIsUserNameLegal(user2));
	return strcmp(user1, user2);
}
static void memcacheUserFree(MemCacheUser user) {
	free(user);
}
static MemCacheLimit memcacheLimitCopy(ConstMemCacheLimit limit) {
	MemCacheLimit copy;
	MEMCACHE_ALLOCATE(int, copy, NULL);

	*(int*)copy = *(int*)limit;
	return copy;
}
static void memcacheLimitFree(MemCacheLimit limit) {
	free(limit);
}
static bool memcacheIsUserExists(MemCache memcache, MemCacheUser user) {
	assert(memcache != NULL);
	if (!memcacheIsUserNameLegal(user)) {
		return false;
	}

	assert(mapContains(memcache->userMemoryLimit, user) ==
			graphIsVertexExists(memcache->userRelations, user));

	return mapContains(memcache->userMemoryLimit, user);
}

static void memcacheIncreaseUserLimit(MemCache memcache, MemCacheUser user, const int inc) {
	assert(memcache != NULL);
	assert(memcacheIsUserNameLegal(user));
	assert(memcacheIsUserExists(memcache, user));
	assert(mapContains(memcache->userMemoryLimit, user));
	MemCacheLimit oldLimit = mapGet(memcache->userMemoryLimit, user);
	*(int*)oldLimit += inc;
}

static int memcacheGetUserLimit(MemCache memcache, MemCacheUser user) {
	assert(memcache != NULL);
	assert(memcacheIsUserNameLegal(user));
	assert(memcacheIsUserExists(memcache, user));
	assert(mapContains(memcache->userMemoryLimit, user));
	return *(int*)mapGet(memcache->userMemoryLimit, user);
}

MemCache memCacheCreate() {
	MemCache memcache;
	MEMCACHE_ALLOCATE(MemCache_t, memcache, NULL);

	memcache->freeBlocks = cacheCreate(
			MEMCACHE_FREE_BLOCK_MAX_SIZE,
			// we should not deallocate block while clearing, we might need it
			memcacheDoNothing,
			// we should not really copy block either, it will be the same block
			memcacheBlockCopy,
			memcacheBlocksCompare,
			memcacheFreeBlockComputeKey);

	memcache->allocatedBlocks = cacheCreate(
			MEMCACHE_ALLOCATED_BLOCK_MODULO,
			// we should not deallocate block while clearing, we might need it
			memcacheDoNothing,
			// we should not really copy block either, it will be the same block
			memcacheBlockCopy,
			memcacheBlocksCompare,
			memcacheAllocatedBlockComputeKey);

	memcache->userRelations = graphCreate(
			(copyGraphVertex)memcacheUserCopy,
			(compareGraphVertex)memcacheUsersCompare,
			(freeGraphVertex)memcacheUserFree);

	memcache->userMemoryLimit = mapCreate(
			(copyMapDataElements)memcacheLimitCopy,
			(copyMapKeyElements) memcacheUserCopy,
			(freeMapDataElements)memcacheLimitFree,
			(freeMapKeyElements)memcacheUserFree,
			(compareMapKeyElements)memcacheUsersCompare);


	if (!memcache->freeBlocks ||
			!memcache->allocatedBlocks ||
			!memcache->userRelations) {
		memCacheDestroy(memcache);
		return NULL;
	}

	return memcache;
}

void memCacheDestroy(MemCache memcache){
	if (memcache == NULL) {
		return;
	}

	// release everything
	MemCacheResult memCachResult = memCacheReset(memcache);
	assert(memCachResult == MEMCACHE_SUCCESS);
	//destroyers
	cacheDestroy(memcache->freeBlocks);
	cacheDestroy(memcache->allocatedBlocks);
	graphDestroy(memcache->userRelations);
	mapDestroy(memcache->userMemoryLimit);
	free(memcache);
}

MemCacheResult memCacheAddUser(MemCache memcache, char* username, int memory_limit) {
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

MemCacheResult memCacheSetBlockMod(MemCache memcache, char* username, void* ptr, char mod){
	if (memcache==NULL){
		return MEMCACHE_NULL_ARGUMENT;
	}
	if (memcacheIsUserExists(memcache, username)==false){
		return MEMCACHE_USER_NOT_FOUND;
	}
	if (!cacheIsIn(memcache->allocatedBlocks, ptr)){
		return MEMCACHE_BLOCK_NOT_ALLOCATED;
	}
	if (strcmp(memcacheBlockGetOwner(ptr),username) != 0){
		return MEMCACHE_PERMISSION_DENIED;
	}
	if (mod != USER && mod != ALL && mod != GROUP){
		return MEMCACHE_INVALID_ARGUMENT;
	}
	*memcacheBlockGetModePointer(ptr) = mod;
	return MEMCACHE_SUCCESS;
}

MemCacheResult memCacheTrust(MemCache memcache, char* username1, char* username2) {
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

	assert(graphAddEdgeResult == GRAPH_SUCCESS ||
			graphAddEdgeResult == GRAPH_EDGE_ALREADY_EXISTS);

	return MEMCACHE_SUCCESS;
}

MemCacheResult memCacheUntrust(MemCache memcache, char* username1, char* username2){
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}
	if (!memcacheIsUserExists(memcache, username1) ||
		!memcacheIsUserExists(memcache, username2)) {
		return MEMCACHE_USER_NOT_FOUND;
	}

	GraphResult removingEdge = graphRemoveDirectedEdge(memcache->userRelations, username1, username2);
	if (removingEdge == GRAPH_OUT_OF_MEMORY){
		return MEMCACHE_OUT_OF_MEMORY;
	}
	assert(removingEdge == GRAPH_SUCCESS || removingEdge == GRAPH_EDGE_DOES_NOT_EXISTS);
	return MEMCACHE_SUCCESS;
}

void* memCacheAllocate(MemCache memcache, char* username, int size){
	// parameters check
	if (memcache == NULL || !memcacheIsUserExists(memcache, username) || size <= 0) {
		return NULL;
	}
	if (memcacheGetUserLimit(memcache, username) < size) {
		// memory limit exceeded;
		return NULL;
	}

	//try to find in cache
	char *ptr = cacheExtractElementByKey(memcache->freeBlocks, memcacheBlockSizeToCacheIndex(size));
	if (ptr == NULL) {
		//allocate and initialize to zero
		ptr = (char*)calloc(sizeof(int) + 1 + 1 + MEMCACHE_USER_NAME_LENGTH + 2 + size + 1, sizeof(char));
		if (ptr == NULL){
			return NULL;
		}
		// move start to user field
		ptr += sizeof(int) + 1 + 1 + MEMCACHE_USER_NAME_LENGTH + 2;
	}
	// set size of block
	*(int*)(ptr - 2 - MEMCACHE_USER_NAME_LENGTH - 1 - 1 - sizeof(int)) = size;
	// set default mode
	*memcacheBlockGetModePointer(ptr) = USER;
	// set owner
	strcpy(memcacheBlockGetOwner(ptr), username);

	CacheResult cacheAddResult = cachePush(memcache->allocatedBlocks, ptr);
	if (cacheAddResult == CACHE_OUT_OF_MEMORY){
		// consistency
		cacheFreeElement(memcache->allocatedBlocks, ptr);
		memcacheFreeBlock(ptr);
		return NULL;
	}

	// decrease limit
	memcacheIncreaseUserLimit(memcache, username, -size);

	assert(cacheAddResult == CACHE_SUCCESS);
	return ptr;
}

MemCacheResult memCacheFree(MemCache memcache, char* username, void* ptr) {
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
	MemCacheUser owner = memcacheBlockGetOwner(ptr);
	switch (mode) {
	case USER:
		if (0 != memcacheUsersCompare(username, owner)) {
			return MEMCACHE_PERMISSION_DENIED;
		}
		break;
	case GROUP:
		if (0 != memcacheUsersCompare(username, owner) &&
				!graphIsDirectedEdgeExists(memcache->userRelations, owner, username)) {
			return MEMCACHE_PERMISSION_DENIED;
		}
		break;
	case ALL:
		// nothing special to check
		break;
	default:
		assert(false);
	}

	int blockSize = memcacheBlockGetSize(ptr);
	memcacheIncreaseUserLimit(memcache, owner, blockSize);
	if (blockSize > MEMCACHE_FREE_BLOCK_MAX_SIZE) {
		// release without removing from cache
		memcacheFreeBlock(ptr);
	} else {
		assert(blockSize <= MEMCACHE_FREE_BLOCK_MAX_SIZE);
		CacheResult cachePushResult = cachePush(memcache->freeBlocks, ptr);
		if (cachePushResult == CACHE_OUT_OF_MEMORY) {
			return MEMCACHE_OUT_OF_MEMORY;
		}
		assert(cachePushResult == CACHE_SUCCESS);
	}
	//remove WITHOUT releasing
	CacheResult cacheFreeResult = cacheFreeElement(memcache->allocatedBlocks, ptr);
	assert(cacheFreeResult == CACHE_SUCCESS);

	return MEMCACHE_SUCCESS;
}

static void *memCacheGetFirstBlock(Cache cache) {
	assert(cache != NULL);
	CACHE_FOREACH(set, cache) {
		assert(set != NULL);
		if (setGetFirst(set) != NULL) {
			return setGetFirst(set);
		}
	}
	return NULL;
}
static void *memCacheGetCurrentBlock(Cache cache) {
	assert(cache != NULL);
	Set cacheCurrentCell = cacheGetCurrent(cache);
	if (!cacheCurrentCell) {
		return NULL;
	}
	return setGetCurrent(cacheCurrentCell);
}
static void *memCacheGetNextBlock(Cache cache) {
	assert(cache != NULL);
	Set cacheCurrentCell = cacheGetCurrent(cache);
	if (!cacheCurrentCell) {
		// cache iterator not set or finished
		return NULL;
	}

	if (setGetNext(cacheCurrentCell)) {
		// we have elements in current cell
		assert(setGetCurrent(cacheCurrentCell) != NULL);
		return setGetCurrent(cacheCurrentCell);
	}

	// no elements in current set
	assert(!setGetCurrent(cacheCurrentCell));
	// skip empty cells
	while (cacheGetNext(cache) != NULL &&
			setGetFirst(cacheGetCurrent(cache)) == NULL);

	assert(cacheGetCurrent(cache) == NULL || setGetFirst(cacheGetCurrent(cache)) != NULL);
	//NULL safe
	return memCacheGetCurrentBlock(cache);
}

void* memCacheGetFirstAllocatedBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return memCacheGetFirstBlock(memcache->allocatedBlocks);
}

void* memCacheGetNextAllocatedBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return memCacheGetNextBlock(memcache->allocatedBlocks);
}

void* memCacheGetCurrentAllocatedBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return memCacheGetCurrentBlock(memcache->allocatedBlocks);
}

void* memCacheGetFirstFreeBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}

	return memCacheGetFirstBlock(memcache->freeBlocks);
}

void* memCacheGetNextFreeBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return memCacheGetNextBlock(memcache->freeBlocks);
}

void* memCacheGetCurrentFreeBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return memCacheGetCurrentBlock(memcache->freeBlocks);
}

static CacheResult memCacheClearBlockCache(Cache cache, int size) {
	assert(cache != NULL && size > 0);
	for (int key = 0; key < size; ++key) {
		MemCacheBlock block;
		while ((block = cacheExtractElementByKey(cache, key)) != NULL) {
			memcacheFreeBlock(block);
		}
	}
	return cacheClear(cache);
}

MemCacheResult memCacheReset(MemCache memcache) {
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}

	GraphResult graphClearResult = graphClear(memcache->userRelations);
	assert(graphClearResult == GRAPH_SUCCESS);
	MapResult mapClearResult = mapClear(memcache->userMemoryLimit);
	assert(mapClearResult == MAP_SUCCESS);

	// remove and release all blocks
	CacheResult allocatedCacheClear =
		memCacheClearBlockCache(memcache->allocatedBlocks, MEMCACHE_ALLOCATED_BLOCK_MODULO);
	assert(allocatedCacheClear == CACHE_SUCCESS);
	CacheResult freeCacheClear =
			memCacheClearBlockCache(memcache->freeBlocks, MEMCACHE_FREE_BLOCK_MAX_SIZE);
	assert(freeCacheClear == CACHE_SUCCESS);

	return MEMCACHE_SUCCESS;
}
