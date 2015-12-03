/*
 * memcache.h
 *
 *  Created on: Nov 30, 2015
 *      Author: Infoshoc_2
 */

#ifndef MEMCACHE_H_
#define MEMCACHE_H_

/** type for defining memory management system **/
typedef struct MemCache_t *MemCache;

typedef enum MemCachResult_t {
	MEMCACHE_SUCCESS,
	MEMCACHE_NULL_ARGUMENT,
	MEMCACHE_USER_NOT_FOUND,
	MEMCACHE_BLOCK_NOT_ALLOCATED,
	MEMCACHE_PERMISSION_DENIED,
	MEMCACHE_OUT_OF_MEMORY,
	MEMCACHE_INVALID_ARGUMENT,
	MEMCACHE_ILLEGAL_USERNAME,
	MEMCACHE_USERNAME_ALREADY_USED
} MemCachResult;

/** Create a new memory management system **/
MemCache memCacheCreate();

void memCacheDestroy(MemCache memcache);

MemCachResult memCacheAddUser(MemCache memcache, const char* const username, int memory_limit);

MemCachResult memCacheSetBlockMod(MemCache memcache, const char* const username, void* ptr, char
mod);

MemCachResult memCacheTrust(MemCache memcache, const char* const username1, const char* const
username2);

MemCachResult memCacheUntrust(MemCache memcache, const char* const username1, const char* const
username2);

void* memCacheAllocate(MemCache memcache, const char* const username, int size);

MemCachResult memCacheFree(MemCache memcache, const char* const username, void* ptr);

void* memCacheGetFirstAllocatedBlock(MemCache memcache);

void* memCacheGetNextAllocatedBlock(MemCache memcache);

void* memCacheGetCurrentAllocatedBlock(MemCache memcache);

void* memCacheGetFirstFreeBlock(MemCache memcache);

void* memCacheGetNextFreeBlock(MemCache memcache);

void* memCacheGetCurrentFreeBlock(MemCache memcache);

MemCachResult memCacheReset(MemCache memcache);
#endif /* MEMCACHE_H_ */
