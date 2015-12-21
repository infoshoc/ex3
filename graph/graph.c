/*
 * graph.c
 *
 *  Created on: Dec 5, 2015
 *      Author: Infoshoc_2
 */

#include "set.h"
#include <stdlib.h>

typedef struct Graph_t {
	CopyGraphVertex copyVertex;
	CompareGraphVertex compareVertex;
	FreeGraphVertex freeVertex;
	Set vertices;
	Set edges;
} Graph_t;

typedef struct GraphEdge_t {
	ConstGraph graph; // just from vertex functions
	ConstGraphVertex from;
	ConstGraphVertex to;
} GraphEdge_t;
typedef const struct GraphEdge_t * const ConstGraphEdge;
typedef struct GraphEdge_t * GraphEdge;

#define GRAPH_ALLOCATE(type, var, error) \
	do { \
		if (NULL == (var = (type*)malloc(sizeof(type)))) { \
			return error; \
		} \
	} while(false)

static GraphEdge graphEdgeCreate(ConstGraph graph, ConstGraphVertex from, ConstGraphVertex to) {
	GraphEdge edge;
	GRAPH_ALLOCATE(GraphEdge_t, edge, NULL);
	edge->graph = graph;

	edge->from = graph->copyVertex(from);
	if (edge->from == NULL) {
		free(edge);
		return NULL;
	}

	edge->to = graph->copyVertex(to);
	if (edge->to == NULL) {
		free(edge);
		graph->freeVertex(edge->from);
		return NULL;
	}
	return edge;
}

static GraphEdge graphEdgeCopy(ConstGraphEdge edge) {
	return graphEdgeCreate(edge->graph, edge->from, edge->to);
}

static void graphEdgeFree(GraphEdge edge) {
	edge->graph->freeVertex(edge->from);
	edge->graph->freeVertex(edge->to);
	free(edge);
}

/**
 * if first from-vertex less then second returns negative number
 * if first from-vertex greater then second returns positive number
 * if from-vertices are equal returns negative number if first to-vertex less
 * then second, positive if first greater then second and zero if edges are equal
 */
static int graphEdgeCompare(ConstGraphEdge edge1, ConstGraphEdge edge2) {
	assert(edge1->graph == edge2->graph);
	int fromDifference = edge1->graph->compareVertex(edge1->from, edge2->from);
	if (0 == fromDifference) {
		return edge1->graph->compareVertex(edge1->to, edge2->to);
	}
	return fromDifference;
}

Graph graphCreate(CopyGraphVertex copyVertex, CompareGraphVertex compareVertex, FreeGraphVertex freeVertex) {
	Graph graph;
	GRAPH_ALLOCATE(Graph_t, graph, NULL);

	graph->copyVertex = copyVertex;
	graph->compareVertex = compareVertex;
	graph->vertices = setCreate(copyVertex, freeVertex, compareVertex);
	graph->edges = setCreate(graphEdgeCopy, graphEdgeFree, graphEdgeCompare);
}

void graphDestroy(Graph graph) {
	//int numberEdges = setGetSize(graph->edges);
	//int numberVertices = setGetSize(graph->vertices);
	setDestroy(graph->vertices);
	setDestroy(graph->edges);
}

GraphResult graphAddVertex(Graph graph, ConstGraphVertex vertex) {
	if (graph == NULL || vertex == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (graphIsVertexExists(graph, vertex)) {
		return GRAPH_VERTEX_ALREADY_EXISTS;
	}
	SetResult setAddResult = setAdd(graph->vertices, vertex);
	if (setAddResult == SET_OUT_OF_MEMORY) {
		return GRAPH_OUT_OF_MEMORY;
	}
	assert(setAddResult == SET_SUCCESS);
	return GRAPH_SUCCESS;
}

GraphResult graphRemoveVertex(Graph graph, ConstGraphVertex vertex){
	if (graph == NULL || vertex == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (graphIsVertexExists(graph, vertex)==false) {
		return GRAPH_VERTEX_DOES_NOT_EXISTS;
	}
	SetResult removing = setRemove(graph->vertices, vertex);
	freeGraphVertex (vertex);
	return GRAPH_SUCCESS;
}

bool graphIsVertexExists(ConstGraph graph, ConstGraphVertex vertex) {
	if (graph == NULL || vertex == NULL) {
		return false;
	}

	return setIsIn(graph->vertices, vertex);
}

GraphResult graphAddDirectedEdge(Graph graph, ConstGraphVertex from, ConstGraphVertex to){
	if (graph == NULL || from == NULL || to == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (graphIsDirectedEdge(graph, from, to)) {
		return GRAPH_EDGE_ALREADY_EXISTS;
	}
	GraphEdge newEdge = graphEdgeCreate(graph, from, to);
	if (newEdge == NULL){
		return GRAPH_OUT_OF_MEMORY;
	}
	SetResult setAddResult = setAdd(graph->edges, newEdge);
	if (setAddResult == SET_OUT_OF_MEMORY) {
		graphEdgeFree(newEdge);
		return GRAPH_OUT_OF_MEMORY;
	}
	assert(setAddResult == SET_SUCCESS);
	return GRAPH_SUCCESS;
}

GraphResult graphRemoveDirectedEdge(Graph graph, ConstGraphVertex from, ConstGraphVertex to) {
	if (graph == NULL || from == NULL || to == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (!graphIsDirectedEdge(graph, from, to)) {
		return GRAPH_EDGE_DOES_NOT_EXISTS;
	}
	GraphEdge edge = graphEdgeCreate(graph, from, to);
	if (edge == NULL) {
		return GRAPH_OUT_OF_MEMORY;
	}
	SetResult setRemoveResult = setRemove(graph->edges, edge);
	assert(setRemoveResult == SET_SUCCESS);
	return GRAPH_SUCCESS;
}

bool graphIsDirectedEdge(Graph graph,  ConstGraphVertex from, ConstGraphVertex to){
	if (graph == NULL || from == NULL || to == NULL) {
			return GRAPH_NULL_ARGUMENT;
		}
		if (!graphIsDirectedEdge(graph, from, to)) {
			return GRAPH_EDGE_DOES_NOT_EXISTS;
		}
		GraphEdge edge = graphEdgeCreate(graph, from, to);
		bool isIn = setIsIn(graph->edges, edge);
		graphEdgeFree(GraphEdge edge);
		return isIn;
}

GraphResult graphClear(Graph graph) {
	if (graph == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}

	SetResult edgesClearResult = setClear(graph->edges);
	SetResult verticesClearResult = setClear(graph->vertices);

	assert(edgesClearResult == SET_SUCCESS &&
			verticesClearResult == SET_SUCCESS);

	return GRAPH_SUCCESS;
}
