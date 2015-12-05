/*
 * memcache.h
 *
 *  Created on: Nov 30, 2015
 *      Author: Infoshoc_2
 */

#ifndef MEMCACHE_H_
#define MEMCACHE_H_


/**
 * Definition of different result types.
 */
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

/**
 * Type for defining memory management system
 */
typedef struct MemCache_t *MemCache;

/**
 * Creates a new memory management system
 *
 * @return A new allocated memory management system or NULL in case of error.
 */
MemCache memCacheCreate();

/**
 * Destroys a memory management system - frees its memory
 *
 * @param memcache - memory management system to destroy
 *
 * @return void
 */
void memCacheDestroy(MemCache memcache);

/**
 * Adds new user to memory management system.
 *
 * @param memcache - memory management system to which users is added
 * @param username - name of new user
 * @param memory_limit - maximal number of bytes availible for user
 *
 * @return
 * 	MEMCACHE_SUCCESS - in case of success
 * 	MEMCACHE_NULL_ARGUMENT - if memcache is NULL
 * 	MEMCACHE_INVALID_ARGUMENT - if memory_limit is not positive
 * 	MEMCACHE_ILLEGAL_USERNAME - if username is not legal
 * 	MEMCACHE_USERNAME_ALREADY_USED - if username already used by other user
 * 	MEMCACHE_OUT_OF_MEMORY - in case of memory lack
 */
MemCachResult memCacheAddUser(MemCache memcache, const char* const username, int memory_limit);

/**
 * Changes permission for block
 *
 * @param memcache - used memory management system
 * @param username - name of user
 * @param ptr - pointer to block
 * @param mod - new access rule 'U', 'G' or 'A' :
 * 	'U' - just owner has access
 * 	'G' - user and trust group have access
 * 	'A' - all users in system have access
 *
 * @return
 * 	MEMCACHE_SUCCESS in case of success
 * 	MEMCACHE_NULL_ARGUMENT if memcache is NULL
 * 	MEMCACHE_USER_NOT_FOUND - if user with name username doesn't exist
 * 	MEMCACHE_BLOCK_NOT_ALLOCATED - if there is no allocated block with address prt
 * 	MEMCACHE_PERMISSION_DENIES - if username is not user who allocated block
 * 	MEMCACHE_INVALID_ARGUMENT - if mode is not legal
 */
MemCachResult memCacheSetBlockMod(MemCache memcache, const char* const username, void* ptr, char mod);

/**
 * Adds username2 to username1's trusted group (if he is already there nothing is done)
 *
 * @param memcache - used memory management system
 * @param username1 - name of user whose trusted group adding to
 * @param username2 - name of user whom adding to trusted group of username1
 *
 * @return
 * 	MEMCACHE_SUCCESS in case of success
 * 	MEMCACHE_NULL_ARGUMENT if memcache is NULL
 * 	MEMCACHE_USER_NOT_FOUND - if user with name username1 or username2 doesn't exist
 * 	MEMCACHE_OUT_OF_MEMORY - in case of memory leak
 */
MemCachResult memCacheTrust(MemCache memcache, const char* const username1, const char* const username2);

/**
 * Removes username2 from username1's trusted group (if he is not there nothing is done)
 *
 * @param memcache - used memory management system
 * @param username1 - name of user whose trusted group removing from
 * @param username2 - name of user who is removed from trusted group of username1
 *
 * @return
 * 	MEMCACHE_SUCCESS - in case of success
 * 	MEMCACHE_NULL_ARGUMENT - if memcache is NULL
 * 	MEMCACHE_USER_NOT_FOUND - if user with name username1 or username2 doesn't exist
 * 	MEMCACHE_OUT_OF_MEMORY - in case of memory leak
 */
MemCachResult memCacheUntrust(MemCache memcache, const char* const username1, const char* const username2);

/**
 * Allocates block of memory of size for username (if he didn't exceed  limit)
 * and returns it's address
 *
 * @param memcache - used memory management system
 * @param username - name of user
 * @param size - requested number of bytes
 *
 * @return address of allocated block or NULL in case of error
 */
void* memCacheAllocate(MemCache memcache, const char* const username, int size);

/**
 * Releases block ptr
 *
 * @param memcache - used memory management system
 * @param username - name of user
 * @param ptr - pointer to block
 *
 * @return
 * 	MEMCACHE_SUCCESS - in case of success
 * 	MEMCACHE_NULL_ARGUMENT - if memcache is NULL
 * 	MEMCACHE_USER_NOT_FOUND - if user with name username doesn't exist
 * 	MEMCACHE_BLOCK_NOT_ALLOCATED - ptr is not points to allocated block
 * 	MEMCACHE_PERMISSION_DENIED - username have no rights to release block
 * 	MEMCACHE_OUT_OF_MEMORY - in case of memory leak
 */
MemCachResult memCacheFree(MemCache memcache, const char* const username, void* ptr);

/**
 * Sets the internal iterator (also called current element) to
 * the first allocated block. The "first" block is the one having the
 * least size and if there is the same size least address.
 *
 * Use this to start iterating over the allocated blocks.
 * Use (To continue iteration use memCacheGetNextAllocatedBlock)
 *
 * @param memcache - used memory management system
 *
 * @return
 * 	NULL if a NULL pointer was sent, the list of blocks is empty or memory lack.
 * 	The first allocated block otherwise
 */
void* memCacheGetFirstAllocatedBlock(MemCache memcache);

/**
 * Advances the internal iterator to the next allocated block and returns it.
 *
 * @param memcache - used memory management system
 *
 * @return
 * 	NULL if reached the end of the cache, or the iterator is at an invalid state
 * 	or a NULL sent as argument
 * 	The next allocated block in case of success
 */
void* memCacheGetNextAllocatedBlock(MemCache memcache);

/**
 * Returns the block pointed by the iterator.
 * @param memcache - used memory management system
 *
 * @return
 *   NULL if the iterator is at an invalid state
 *   or a NULL sent as argument
 *   The current allocated block in case of success
 */
void* memCacheGetCurrentAllocatedBlock(MemCache memcache);

/**
 * Sets the internal iterator (also called current element) to
 * the first free block. The "first" block is the one having the
 * least size and if there is the same size least address.
 *
 * Use this to start iterating over the free blocks.
 * Use (To continue iteration use memCacheGetNextFreeBlock)
 *
 * @param memcache - used memory management system
 *
 * @return
 * 	NULL if a NULL pointer was sent, the list of blocks is empty or memory lack.
 * 	The first free block otherwise
*/
void* memCacheGetFirstFreeBlock(MemCache memcache);

/**
 * Advances the internal iterator to the next free block and returns it.
 *
 * @param memcache - used memory management system
 *
 * @return
 * 	NULL if reached the end of the cache, or the iterator is at an invalid state
 * 	or a NULL sent as argument
 * 	The next free block in case of success
 */
void* memCacheGetNextFreeBlock(MemCache memcache);

/**
 * Returns the block pointed by the iterator.
 * @param memcache - used memory management system
 *
 * @return
 *   NULL if the iterator is at an invalid state
 *   or a NULL sent as argument
 *   The current free block in case of success
 */
void* memCacheGetCurrentFreeBlock(MemCache memcache);

/**
 * Resets memory management system. Resets user group and releases all blocks of memory
 *
 * @param memcache - used memory management system
 *
 * @return
 * 	MEMCACHE_SUCCESS - in case of success
 * 	MEMCACHE_NULL_ARGUMENT - if memcache is NULL
 */
MemCachResult memCacheReset(MemCache memcache);

/*!
* Macro for iterating over a allocated blocks.
* Declares a new iterator for the loop.
*/
#define MEMCACHE_ALLOCATED_FOREACH(type,iterator,memcache) \
	for(type iterator = memcacheGetFirstAllocatedBlock(memcache) ; \
		iterator ;\
		iterator = memcacheGetNextAllocatedBlock(memcache))

/*!
* Macro for iterating over a free blocks.
* Declares a new iterator for the loop.
*/
#define MEMCACHE_ALLOCATED_FOREACH(type,iterator,memcache) \
	for(type iterator = memcacheGetFirstFreeBlock(memcache) ; \
		iterator ;\
		iterator = memcacheGetNextFreeBlock(memcache))

#endif /* MEMCACHE_H_ */
