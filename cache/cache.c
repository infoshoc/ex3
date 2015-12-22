#include "cache.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_INVALID_ITERATOR_INDEX 1
typedef struct cache_t {
	ComputeCacheKey computeKey;
	Set *container;
	int cache_size;
	int iteratorIndex;
} cache_t;

#define CACHE_ALLOCATE(type, var, error) \
	do { \
		if (NULL == (var = (type*)malloc(sizeof(type)))) { \
			return error; \
		} \
	} while(false)

#define CACHE_CONTAINER_FOREACH(index, cache) \
		for (int index = 0; index < cache->cache_size; ++index)

/**
 * checks if index of cell is in range
 */
inline static bool cacheIsKeyCorrect(const Cache cache, const int key) {
	assert(cache != NULL);
	return 0 <= key && key < cache->cache_size;
}
/**
 * creates cache with special size
 */
Cache cacheCreate(
    int size,
    FreeCacheElement free_element,
    CopyCacheElement copy_element,
    CompareCacheElements compare_elements,
    ComputeCacheKey compute_key) {
	if (size <= 0 || !free_element || !copy_element || !compare_elements || !compute_key) {
		return NULL;
	}
	//memory allocation
	Cache cache;
	CACHE_ALLOCATE(cache_t, cache, NULL);

	//initialization
	cache->cache_size = size;
	cache->iteratorIndex = CACHE_INVALID_ITERATOR_INDEX;
	cache->container = (Set*)malloc(sizeof(*cache->container) * size);
	if (cache->container == NULL) {
		cacheDestroy(cache);
		return NULL;
	}
	CACHE_CONTAINER_FOREACH(i, cache) {
		cache->container[i] = setCreate(copy_element, free_element, compare_elements);
		if (cache->container[i] == NULL) {
			cacheDestroy(cache);
			return NULL;
		}
	}
	return cache;
}

CacheResult cachePush(Cache cache, CacheElement element) {
	// TODO make generic
	if (cache == NULL || element == NULL) {
		return CACHE_ILLEGAL_ARGUMENT;
	}

	int cellIndex = computeKey(element);
	if (!cacheIsKeyCorrect(cache, cellIndex)) {
		return CACHE_OUT_OF_RANGE;
	}

	if (listInsertFirst(cache->container[cellIndex], element) == LIST_OUT_OF_MEMORY) {
		return CACHE_OUT_OF_MEMORY;
	}

	return CACHE_SUCCESS;
}

CacheResult cacheFreeElement(Cache cache, CacheElement element) {
	if (cache == NULL || element == NULL) {
		return CACHE_NULL_ARGUMENT;
	}

	int key = cache->computeKey(element);
	if (!cacheIsKeyCorrect(cache, key)){
		return CACHE_ITEM_DOES_NOT_EXIST;
	}
	SetResult removeResult = setRemove(cache->container[key], element);
	if (removeResult == SET_ITEM_DOES_NOT_EXIST) {
		return CACHE_ITEM_DOES_NOT_EXIST;
	}
	assert(removeResult == SET_SUCCESS);
	return CACHE_SUCCESS;
}

CacheResult cacheGet(Cache cache, int index, Orange* org) {
	if (cache == NULL) {
		return CACHE_ILLEGAL_ARGUMENT;
	}

	int cellIndex = cacheGetCellIndexForOrangeSize(index);
	if (!isCellIndexCorrect(cache, cellIndex)) {
		return CACHE_OUT_OF_RANGE;
	}

	if (listGetSize(cache->container[cellIndex]) == 0) {
		return CACHE_NO_ELEMENTS_IN_CELL;
	}

	if ((*org = orangeCopy(listGetFirst(cache->container[cellIndex]))) == NULL) {
		return CACHE_OUT_OF_MEMORY;
	}
	cacheFreeOrange(cache, index);
	return CACHE_SUCCESS;
}

bool cacheIsIn(Cache cache, CacheElement element) {
	if (cache == NULL || element == NULL) {
		// null argument
		return false;
	}
	int key = cache->computeKey(element);
	if (!cacheIsKeyCorrect(cache, key)) {
		// out of range
		return NULL;
	}
	return setIsIn(cache->container[key], element);
}

List cacheGetFirst(Cache cache) {
	assert(cache != NULL);
	cache->iterator = cache->container[0];
	return cache->iterator;
}

Set cacheGetNext(Cache cache) {
	if (cache == NULL ||
			cache->iteratorIndex == cacheGetNext) {
		return NULL;
	}

	if (cache->iteratorIndex + 1 == cache->cache_size) {
		cache->iteratorIndex = CACHE_INVALID_ITERATOR_INDEX;
		return NULL;
	}
	++cache->iteratorIndex;

	return cache->container[cache->iteratorIndex];
}

List cacheGetCurrent(Cache cache) {
	if (cache == NULL){
		return NULL;
	}
	return cache->iterator;
}

CacheResult cacheClear(Cache cache) {
	if (cache == NULL) {
		return CACHE_NULL_ARGUMENT;
	}

	CACHE_CONTAINER_FOREACH(i, cache) {
		SetResult setClearResult = setClear(cache->container[i]);
		assert(setClearResult == SET_SUCCESS);
	}

	return CACHE_SUCCESS;
}

void cacheDestroy(Cache cache) {
	if (cache == NULL) {
		return;
	}

	for (int i = 0; i < cache->cache_size; ++i) {
		listDestroy(cache->container[i]);
	}
	free(cache->container);
	free(cache);
}
