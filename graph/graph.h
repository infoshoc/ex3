/*
 * graph.h
 *
 *  Created on: Dec 4, 2015
 *      Author: Infoshoc_2
 */

#ifndef GRAPH_H_
#define GRAPH_H_

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

Graph GraphCreate(copyGraphVertex, compareGraphVertex, freeGraphVertex);
void GraphDestroy(Graph);
GraphResult GraphAddVertex(Graph, ConstGraphVertex);
GraphResult GraphRemoveVertex(Graph, ConstGraphVertex);
bool GraphIsVertexExists(Graph, ConstGraph, Vertex);
GraphResult GraphAddDirectedEdge(Graph, ConstGraphVertex from, ConstGraphVertex to);
GraphResult GraphRemoveDirectedEdge(Graph, ConstGraphVertex from, ConstGraphVertex to);
bool GraphIsDirectedEdge(Graph,  ConstGraphVertex from, ConstGraphVertex to);

#endif /* GRAPH_H_ */

