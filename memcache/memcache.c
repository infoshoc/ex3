/*
 * memcache.c
 *
 *  Created on: Nov 30, 2015
 *      Author: Infoshoc_2
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memcache.h"
#include "cache.h"
#include "graph.h"
#include "map.h"
#include "set.h"

#define MEMCACHE_FREE_BLOCK_MAX_SIZE (265)
#define MEMCACHE_ALLOCATED_BLOCK_MODULO (1<<10)
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
typedef const char ConstMemCacheUser[MEMCACHE_USER_NAME_LENGTH+1];
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
static int memcacheBlocksCompare(MemCacheBlock a, MemCacheBlock b) {
	// in order to escape overflow in difference
	return (uintptr_t)a < (uintptr_t)b ? -1 : (uintptr_t*)a == (uintptr_t*)b ? 0 : 1;
}
static inline int memcacheBlockGetSize(MemCacheBlock block) {
	return *(int*)((char*)block-2-MEMCACHE_USER_NAME_LENGTH-1-sizeof(int));
}
static inline MemCacheBlockMode memcacheBlockGetMode(MemCacheBlock block) {
	return *((char*)block-2-MEMCACHE_USER_NAME_LENGTH-1);
}
/** Function returns owner of block (NOT COPY) */
static ConstMemCacheUser memcacheBlockGetOwner(MemCacheBlock block) {
	return (char*)block-2-MEMCACHE_USER_NAME_LENGTH;
}
static int memcacheAvailibleBlockComputeKey(MemCacheBlock block) {
	return (uintptr_t)block % MEMCACHE_ALLOCATED_BLOCK_MODULO;
}
static int memcacheAllocatedBlockComputeKey(MemCacheBlock block) {
	return memcacheBlockGetSize(block)-1;
}
static bool memcacheIsUserNameLegal(ConstMemCacheUser user) {
	if (user == NULL) {
		return false;
	}
	if (user[MEMCACHE_USER_NAME_LENGTH+1] != '\0') {
		return false;
	}
	return strlen(user) == MEMCACHE_USER_NAME_LENGTH;
}
static MemCacheUser memcacheUserCopy(ConstMemCacheUser user) {
	assert(memcacheIsUserNameLegal(user));
	MemCacheUser copy;
	MEMCACHE_ALLOCATE(ConstMemCacheUser, copy, NULL);

	strcpy(copy, user);
	return copy;

}
static int memcacheUsersCompare(ConstMemCacheUser user1, ConstMemCacheUser user2) {
	assert(memcacheIsUserNameLegal(user1) && memcacheIsUserNameLegal(user2));
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
static bool memcacheIsUserExists(MemCache memcache, ConstMemCacheUser user) {
	assert(memcache != NULL);
	if (!memcacheIsUserNameLegal(user)) {
		return false;
	}
	assert(mapContains(memcache->userMemoryLimit, user) ==
			graphIsVertexExists(memcache->userRelations, user));

	return mapContains(memcache->userMemoryLimit, user);
}

static void memcacheIncreaseUserLimit(MemCache memcache, ConstMemCacheUser user, const int inc) {
	assert(memcache != NULL);
	assert(memcacheIsUserNameLegal(user));
	assert(memcacheIsUserExists(memcache, user));
	assert(mapContains(memcache->userMemoryLimit, user));
	MemCacheLimit oldLimit = mapGet(memcache->userMemoryLimit, user);
	*(int*)oldLimit += inc;
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
			memcacheAvailibleBlockComputeKey);

	memcache->allocatedBlocks = cacheCreate(
			MEMCACHE_FREE_BLOCK_MAX_SIZE,
			// we should not deallocate block while clearing, we might need it
			memcacheDoNothing,
			// we should not really copy block either, it will be the same block
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

void memCacheDestroy(MemCache memcache){
	cacheDestroy(memcache->freeBlocks);
	cacheDestroy(memcache->allocatedBlocks);
	graphDestroy(memcache->userRelations);
	mapDestroy(memcache->userMemoryLimit);
	setDestroy(memcache->allAllocatedBlocks);
	setDestroy(memcache->allFreeBlocks);
}

MemCachResult memCacheAddUser(MemCache memcache, char* username, int memory_limit) {
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

MemCachResult memCacheSetBlockMod(MemCache memcache, char* username, void* ptr, char mod){
	if (memcache==NULL){
		return MEMCACHE_NULL_ARGUMENT;
	}
	if (memcacheIsUserExists(memcache, username)==false){
		return MEMCACHE_USER_NOT_FOUND;
	}
	if (!setIsIn(memcache->allAllocatedBlocks, ptr)){
		return MEMCACHE_BLOCK_NOT_ALLOCATED;
	}
	if (memcacheBlockGetOwner(ptr) != username){
		return MEMCACHE_PERMISSION_DENIED;
	}
	if (mod !='U'
			&& mod != 'A'
					&& mod != 'G'){
		return MEMCACHE_INVALID_ARGUMENT;
	}

	return MEMCACHE_SUCCESS;
}

MemCachResult memCacheTrust(MemCache memcache, char* username1, char* username2) {
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

MemCachResult memCacheUntrust(MemCache memcache, char* username1, char* username2){
	if (memcache == NULL) {
		return MEMCACHE_NULL_ARGUMENT;
	}
	if (!memcacheIsUserExists(memcache, username1) ||
		!memcacheIsUserExists(memcache, username2)) {
		return MEMCACHE_USER_NOT_FOUND;
	}
	GrarhResult removingEdge = graphRemoveDirectedEdge(memcache->userRelations, username1, username2);
	if (removingEdge == GRAPH_OUT_OF_MEMORY){
		return MEMCACHE_OUT_OF_MEMORY;
	}
	assert(removingEdge == GRAPH_SUCCESS);
	return MEMCACHE_SUCCESS;
}

void* memCacheAllocate(MemCache memcache, char* username, int size){
	MEMCACHE_ALLOCATE(type, var, NULL);

}

MemCachResult memCacheFree(MemCache memcache, char* username, void* ptr) {
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
	case ALL:
		// nothing special to do
		break;
	default:
		assert(false);
	}

	int blockSize = memcacheBlockGetSize(ptr);
	memcacheIncreaseUserLimit(owner, blockSize);
	if (blockSize > MEMCACHE_FREE_BLOCK_MAX_SIZE) {
		// release without removing from cache
		memcacheFreeBlock(ptr);
	} else {
		assert(blockSize <= MEMCACHE_FREE_BLOCK_MAX_SIZE);
		cachePush(memcache->freeBlocks, ptr);
	}
	//remove WITHOUT releasing
	CacheResult cacheFreeResult = cacheFreeElement(memcache->allocatedBlocks, ptr);
	assert(cacheFreeResult == CACHE_SUCCESS);

	return MEMCACHE_SUCCESS;
}

void* memCacheGetFirstAllocatedBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return setGetFirst (memcache->allAllocatedBlocks);
}

void* memCacheGetNextAllocatedBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return setGetNext(memcache->allAllocatedBlocks);
}

void* memCacheGetCurrentAllocatedBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return setGetCurrent (memcache->allAllocatedBlocks);
}

void* memCacheGetFirstFreeBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return setGetFirst (memcache->allFreeBlocks);
}

void* memCacheGetNextFreeBlock(MemCache memcache) {
	if (memcache == NULL) {
		return NULL;
	}
	return setGetNext(memcache->allFreeBlocks);
}

void* memCacheGetCurrentFreeBlock(MemCache memcache){
	if (memcache == NULL) {
		return NULL;
	}
	return setGetCurrent(memcache->allFreeBlocks);
}

static CacheResult memCacheClearBlockCache(Cache cache, int size) {
	assert(cache != NULL && size > 0);
	for (int key = 0; key < size; ++key) {
		MemCacheBlock block;
		while ((block = cacheExtractElementByKey(cache, size)) != NULL) {
			memcacheFreeBlock(block);
		}
	}
	return cacheClear(cache);
}

void memCacheReset(MemCache memcache) {
	graphClear(memcache->userRelations);
	mapClear(memcache->userMemoryLimit);

	// remove without releasing
	setClear(memcache->allAllocatedBlocks);
	setClear(memcache->allFreeBlocks);

	// remove and release all blocks
	memCacheClearBlockCache(memcache->allocatedBlocks, MEMCACHE_ALLOCATED_BLOCK_MODULO);
	memCacheClearBlockCache(memcache->freeBlocks, MEMCACHE_FREE_BLOCK_MAX_SIZE);
}
