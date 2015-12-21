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

typedef GraphVertex(*copyGraphVertex)(GraphVertex);
typedef int(*compareGraphVertex)(GraphVertex, GraphVertex);
typedef void(*freeGraphVertex)(GraphVertex);

Graph graphCreate(copyGraphVertex copyVertex, compareGraphVertex compareVertex, freeGraphVertex freeVertex);

void graphDestroy(Graph graph);
GraphResult graphAddVertex(Graph graph, GraphVertex vertex);
GraphResult graphRemoveVertex(Graph graph, GraphVertex vertex);
bool graphIsVertexExists(ConstGraph graph, GraphVertex vertex);
GraphResult graphAddDirectedEdge(Graph graph, GraphVertex from, GraphVertex to);
GraphResult graphRemoveDirectedEdge(Graph graph, GraphVertex from, GraphVertex to);
bool graphIsDirectedEdgeExists(Graph graph,  GraphVertex from, GraphVertex to);
GraphResult graphClear(Graph graph);

#endif /* GRAPH_H_ */

