#include "cache.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * returns number of cell for oranges of special size
 */
inline static int cacheGetCellIndexForOrangeSize(const int size) {
	return size-1;
}
/**
 * checks if index of cell is in range
 */
inline static bool isCellIndexCorrect(const Cache cache, const int index) {
	assert(cache != NULL);
	return 0 <= index && index < cache->cache_size;
}
/**
 * finds index where iterator is standing in container or cache_size if it is NULL;
 */
static int findIteratorIndex(const Cache cache) {
	assert(cache != NULL);
	int cellIndex;
	for (cellIndex = 0; cellIndex < cache->cache_size &&
		cache->container[cellIndex] != cache->iterator; ++cellIndex);
	return cellIndex;
}
/**
 * creates cache with special size
 */
static Cache cacheCreateWithSize(const unsigned int cacheSize) {
	assert(cacheSize > 0);
	//memory allocation
	Cache cache = (Cache) malloc(sizeof(*cache));
	if (cache == NULL) {
		return NULL;
	}

	//initialization
	cache->cache_size = cacheSize;
	cache->iterator = NULL;
	cache->container = (List*) malloc(cache->cache_size * sizeof(*cache->container));
	if (cache->container == NULL) {
		free(cache);
		return NULL;
	}
	return cache;
}
/**
 * checks if company delivers current orange
 */
static bool cacheIsOrangeDeliveredBy(const Orange orange, const char * const company){
	assert(orange != NULL && company != NULL);
	char* const * foodCompanies = orangeGetFoodCompanies(orange);
	int j;
	for (j = 0; j < orangeGetNumberOfFoodCompanies(orange) &&
		strcmp(foodCompanies[j], company) != 0; ++j);
	return j < orangeGetNumberOfFoodCompanies(orange);
}
/**
 * checks if orange is expired
 */
inline static bool cacheIsOrangeExpired(const Orange orange) {
	assert(orange != NULL);
	return orangeGetExpirationMonth(orange) < CURRENT_MONTH;
}

Cache cacheCreate() {
	Cache cache = cacheCreateWithSize(ORANGE_CACHE_SIZE);
	if (cache == NULL) {
		return NULL;
	}
	for (int i = 0; i < cache->cache_size; ++i) {
		cache->container[i] = listCreate();
	}
	return cache;
}

Cache cacheCopy(Cache src) {
	if (src == NULL){
		return NULL;
	}
	Cache cacheCopied = cacheCreateWithSize(src->cache_size);
	if (cacheCopied == NULL){
		return NULL;
	}

	for (int i = 0; i < src->cache_size; ++i){
		cacheCopied->container[i] = listCopy(src->container[i]);
		if (cacheCopied->container[i] == NULL) {
			cacheDestroy (cacheCopied);
			return NULL;
		}
	}
	return cacheCopied;
}

CacheResult cachePush(Cache cache, Orange orange) {
	if (cache == NULL || orange == NULL) {
		return CACHE_ILLEGAL_ARGUMENT;
	}

	int cellIndex = cacheGetCellIndexForOrangeSize(orangeGetSize(orange));
	if (!isCellIndexCorrect(cache, cellIndex)) {
		return CACHE_OUT_OF_RANGE;
	}

	if (listInsertFirst(cache->container[cellIndex], orange) == LIST_OUT_OF_MEMORY) {
		return CACHE_OUT_OF_MEMORY;
	}

	return CACHE_SUCCESS;
}

CacheResult cacheFreeOrange(Cache cache, int index) {
	if (cache == NULL) {
		return CACHE_ILLEGAL_ARGUMENT;
	}

	int cellIndex = cacheGetCellIndexForOrangeSize(index);
	if (!isCellIndexCorrect(cache, cellIndex)){
		return CACHE_OUT_OF_RANGE;
	}
	if (listGetSize(cache->container[cellIndex]) == 0){
		return CACHE_NO_ELEMENTS_IN_CELL;
	}
	listGetFirst(cache->container[cellIndex]);
	listRemoveCurrent(cache->container[cellIndex]);
	if (listGetSize(cache->container[cellIndex]) == 0){
		return CACHE_SUCCESS_LIST_EMPTY;
	}
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

List cacheGetFirst(Cache cache) {
	assert(cache != NULL);
	cache->iterator = cache->container[0];
	return cache->iterator;
}

List cacheGetNext(Cache cache) {
	assert(cache != NULL);
	if (cache->iterator == NULL) {
		return NULL;
	}

	int cellIndex = findIteratorIndex(cache);

	if (cellIndex+1 == cache->cache_size) {
		cache->iterator = NULL;
		return NULL;
	}
	return cache->iterator = cache->container[cellIndex+1];
}

List cacheGetCurrent(Cache cache) {
	if (cache == NULL){
		return NULL;
	}
	return cache->iterator;
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

List cacheGetOrangesOfCompanyFromCell(Cache cache, char* company,int index){
	if (cache == NULL || company == NULL){
		return NULL;
	}

	int cellIndex = cacheGetCellIndexForOrangeSize(index);
	if (!isCellIndexCorrect(cache, cellIndex)){
		return NULL;
	}
	List orangesToCell = listCreate();
	if (orangesToCell == NULL) {
		return NULL;
	}
	//searcing for oranges in cell
	LIST_FOREACH(Orange, orange, cache->container[cellIndex]) {
		if (cacheIsOrangeExpired(orange)) {
			//skip expired orange
			continue;
		}

		if (!cacheIsOrangeDeliveredBy(orange, company)) {
			// can not get the orange from the company
			continue;
		}
		if (orangeSetFoodCompanyForDelivery(orange, company) != ORANGE_SUCCESS) {
			listDestroy(orangesToCell);
			return NULL;
		}

		if (listInsertFirst(orangesToCell, orange) != LIST_SUCCESS){
			listDestroy(orangesToCell);
			return NULL;
		}
	}

	// removing found oranges
	for (bool removed = true; removed; ) {
		removed = false;
		LIST_FOREACH(Orange, orange, cache->container[cellIndex]) {
			if (!cacheIsOrangeExpired(orange) && cacheIsOrangeDeliveredBy(orange, company)) {
				listRemoveCurrent(cache->container[cellIndex]);
				removed = true;
			}
		}
	}
	return orangesToCell;
}

List cacheGetAllOrangesOfCompany(Cache cache, char* company) {
	if (cache == NULL || company == NULL) {
		return NULL;
	}

	List listOfAllFreshOrangesOfCompany = listCreate();
	if (listOfAllFreshOrangesOfCompany == NULL) {
		return NULL;
	}

	for (int i = 1; i <= cache->cache_size; ++i) {
		List oranges = cacheGetOrangesOfCompanyFromCell(cache, company, i);
		LIST_FOREACH(Orange, orange, oranges) {
			listInsertFirst(listOfAllFreshOrangesOfCompany, orange);
		}
		listDestroy(oranges);
	}

	return listOfAllFreshOrangesOfCompany;
}

List cacheDeliverOrangesWithBiggestWeight(Cache cache) {
	assert(cache != NULL);
	char *foodCompanyMax = NULL;
	int weightMax = 0;

	for (int i=0; i<cache->cache_size; i++){
		LIST_FOREACH(Orange, orange, cache->container[i]) {
			char* const * foodCompanies = orangeGetFoodCompanies(orange);
			for (int k = 0; k < orangeGetNumberOfFoodCompanies(orange); k++){
				Cache copyOfCache = cacheCopy(cache);
				if (copyOfCache == NULL) {
					free(foodCompanyMax);
					return NULL;
				}
				List allOranges = cacheGetAllOrangesOfCompany(copyOfCache, foodCompanies[k]);
				cacheDestroy(copyOfCache);
				if (allOranges == NULL) {
					free(foodCompanyMax);
					return NULL;
				}
				int weight = 0;
				LIST_FOREACH(Orange, orangeOfCompany, allOranges) {
					weight += orangeGetSize(orangeOfCompany);
				}
				listDestroy(allOranges);
				if (weight > weightMax ||
						(weight == weightMax &&
								(foodCompanyMax != NULL &&
								strcmp(foodCompanies[k], foodCompanyMax) > 0))) {
					weightMax = weight;
					free(foodCompanyMax);
					if (NULL == (foodCompanyMax = (char*)malloc(strlen(foodCompanies[k])+1))) {
						cacheDestroy(copyOfCache);
						return NULL;
					}
					strcpy(foodCompanyMax, foodCompanies[k]);
				}
			}

		}
	}
	List orangesWithBiggestWeight = cacheGetAllOrangesOfCompany(cache, foodCompanyMax);
	free(foodCompanyMax);
	return orangesWithBiggestWeight;
}
