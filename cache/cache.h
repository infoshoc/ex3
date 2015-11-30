/*
 * cache.h
 *  
 * Header for generic cache ADT.
 * Created on: Nov 13, 2015
 */

#ifndef CACHE_H_
#define CACHE_H_

#include "set.h"
#include <stdbool.h>

/**
 * Types defintion for the generic implementation.
 */
typedef void* CacheElement;
typedef void (*FreeCacheElement)(CacheElement);
typedef CacheElement (*CopyCacheElement)(CacheElement);
typedef int (*CompareCacheElements)(CacheElement, CacheElement);
typedef int (*ComputeCacheKey)(CacheElement);

/**
 * Defintion of different result types.
 */

typedef enum CacheResult_t {
	CACHE_SUCCESS,
	CACHE_NULL_ARGUMENT,
	CACHE_OUT_OF_RANGE,
	CACHE_NO_ITEMS_IN_CELL,
	CACHE_ITEM_ALREADY_EXISTS,
	CACHE_ITEM_DOES_NOT_EXIST,
	CACHE_OUT_OF_MEMORY,

} CacheResult;

typedef struct cache_t* Cache;

/**
 * Creates a new cache of elements.
 *
 * @param size - number of cells in cache container.
 * @param destroy-element - callback to be called for destroying an element.
 * @param compare_elements - callback to be called for comparing between elements.
 * @param compute_key - callback to be called for computing the key for an 
 * element (i.e., the index of the set it ought to be added to in the
 * container).
 *
 * @return A new allocated cache, or NULL in case of error.
 */
Cache cacheCreate(
    int size,
    FreeCacheElement free_element,
    CopyCacheElement copy_element,
    CompareCacheElements compare_elements,
    ComputeCacheKey compute_key);

/**
 * Adds an element to the cache.
 * 
 * @param cache - cache to add the element to.
 * @param element - element to be added.
 *
 * @return Result code.
 */
CacheResult cachePush(Cache cache, CacheElement element);

/**
 * Removes an element from the cache and destroyes it.
 *
 * @param cache - cache to remove the element from.
 * @param element - element to remove from cache. 
 *
 * @return Result code.
 */
CacheResult cacheFreeElement(Cache cache, CacheElement element);

/**
 * Removes an element with the specified key from the cache, and returns it to
 * the user.
 *
 * @param cache - cache to remove the element from.
 * @param key - key associated with the element removed.
 *
 * @return pointer to extracted element if succeeds, NULL otherwise.
 */
CacheElement cacheExtractElementByKey(Cache cache, int key);

/**
 * Checks whether an element exisdts in the cache.
 *
 * @param cache - cache to search.
 * @param element - element to find.
 *
 * @return true if element found, false otherwise.
 */
bool cacheIsIn(Cache cache, CacheElement element);

/**
 * Sets the internal iterator to the first available cell and retrieves it.
 *
 * @param cache - cache to iterate.
 *
 * @return NULL if a NULL pointer was passed or the cache is empty. The first
 * available cell of the cache otherwise.
 */
Set cacheGetFirst(Cache cache);


/**
 * Advances the caches's iterator to the next available cell in the cache and
 * returns it.
 *
 * @param cache - cache to iterate.
 *
 * @return NULL if reached the end of the cache, or the iterator is at an invalid
 * state or NULL passed as an argument. The next cell on the cache otherwise.
 */
Set cacheGetNext(Cache cache);


/**
 * Returns the current cell (pointed by the iterator)
 *
 * @param cache - cache to iterate.
 *
 * @return NULL if the iterator is at an invalid state or a NULL passed as an 
 * argument. The current cell on the cache in case of success.
 */
Set cacheGetCurrent(Cache cache);

/**
 * Clears a cache - frees its all elements.
 * 
 * @param cache - cache to clear.
 *
 * @return Result code.
 */
CacheResult cacheClear(Cache cache);

/**
 * Destroys a cache - frees its memory.
 * 
 * @param cache - cache to destroy.
 *
 * @return void.
 */
void cacheDestroy(Cache cache);


/**
 * Macro for iterating over a cache.
 *
 * Declares a new variable to hold each cell of the cache.
 * Use this macro for iterating through the cache's cells conveniently.
 * Note that this macro modifies the internal iterator.
 * For example:
 * void findEmptyCells(Cache cacheOfElements) {
 *   CACHE_FOREACH(cell, cacheOfElements) {
 *     if(setGetSize(cell) == 0) {
         printf("found empty cell\n");
         return;
       }
 *   }
 * }
 * @endcode
 *
 * @param iterator - name of variable to hold the set in the container cell of
 * the current iteration.
 * @param cache - cache to iterate over.
*/
#define CACHE_FOREACH(iterator,cache) \
	for(Set iterator = cacheGetFirst(cache) ; \
		iterator ;\
		iterator = cacheGetNext(cache))

#endif /* CACHE_H_ */
