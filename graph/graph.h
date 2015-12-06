#ifndef GRAPH_H_
#define GRAPH_H_

/*
 * graph.h
 *
 * Header for generic graph ADT
 *
 */


#include <stdbool.h>

typedef struct Graph_t *Graph;
typedef const struct Graph_t * const ConstGraph;

typedef enum GraphResult_t {
	GRAPH_SUCCESS,
	GRAPH_OUT_OF_MEMORY,
	GRAPH_NULL_ARGUMENT,
	GRAPH_VERTEX_ALREADY_EXISTS,
	GRAPH_VERTEX_DOES_NOT_EXISTS,
	GRAPH_EDGE_ALREADY_EXISTS,
	GRAPH_EDGE_DOES_NOT_EXISTS
} GraphResult;

typedef void * GraphVertex;
typedef const void * const ConstGraphVertex;

typedef GraphVertex(*copyGraphVertex)(ConstGraphVertex);
typedef int(*compareGraphVertex)(ConstGraphVertex, ConstGraphVertex);
typedef void(*freeGraphVertex)(ConstGraphVertex);

Graph graphCreate(copyGraphVertex copyVertex, compareGraphVertex compareVertex, freeGraphVertex freeVertex);
void graphDestroy(Graph graph);
GraphResult graphAddVertex(Graph graph, ConstGraphVertex vertex);
bool graphIsVertexExists(ConstGraph graph, ConstGraphVertex vertex);
GraphResult graphAddDirectedEdge(Graph graph, ConstGraphVertex from, ConstGraphVertex to);
GraphResult graphRemoveDirectedEdge(Graph graph, ConstGraphVertex from, ConstGraphVertex to);
bool graphIsDirectedEdge(Graph graph,  ConstGraphVertex from, ConstGraphVertex to);
GraphResult graphClear(Graph graph);

#endif /* GRAPH_H_ */

