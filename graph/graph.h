#ifndef GRAPH_H_
#define GRAPH_H_

/*
 * graph.h
 *
 * Header for generic graph ADT
 *
 */


#include <stdbool.h>

typedef struct graph_t *Graph;
typedef const struct graph_t * const ConstGraph;

typedef enum GraphResult_t {
	GRAPH_SUCCESS,
	GRAPH_OUT_OF_MEMORY,
	GRAPH_NULL_ARGUMENT,
	GRAPH_USER_ALREADY_EXISTS,
	GRAPH_USER_DOES_NOT_EXISTS,
	GRAPH_EDGE_ALREADY_EXISTS,
	GRAPH_EDGE_DOES_NOT_EXISTS
} GraphResult;

typedef void * GraphVertex;
typedef const void * const ConstGraphVertex;

typedef GraphVertex(*copyGraphVertex)(ConstGraphVertex);
typedef int(*compareGraphVertex)(ConstGraphVertex, ConstGraphVertex);
typedef void(*freeGraphVertex)(ConstGraphVertex);

Graph graphCreate(copyGraphVertex, compareGraphVertex, freeGraphVertex);
void graphDestroy(Graph);
GraphResult graphAddVertex(Graph, ConstGraphVertex);
GraphResult graphRemoveVertex(Graph, ConstGraphVertex);
bool graphIsVertexExists(Graph, ConstGraph, ConstGraphVertex);
GraphResult graphAddDirectedEdge(Graph, ConstGraphVertex from, ConstGraphVertex to);
GraphResult graphRemoveDirectedEdge(Graph, ConstGraphVertex from, ConstGraphVertex to);
bool graphIsDirectedEdge(Graph,  ConstGraphVertex from, ConstGraphVertex to);
GraphResult graphClear(Graph);

#endif /* GRAPH_H_ */

