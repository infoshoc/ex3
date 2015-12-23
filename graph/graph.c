/*
 * graph.c
 *
 *  Created on: Dec 5, 2015
 *      Author: Infoshoc_2
 */

#include "set.h"
#include "graph.h"
#include "assert.h"
#include <stdlib.h>

typedef struct Graph_t {
	copyGraphVertex copyVertex;
	compareGraphVertex compareVertex;
	freeGraphVertex freeVertex;
	Set vertices;
	Set edges;
} Graph_t;

/** Struct for directed edges */
typedef struct GraphEdge_t {
	Graph graph; // just for vertex functions
	GraphVertex from;
	GraphVertex to;
} GraphEdge_t, *GraphEdge;

/**
 * Safe allocation of an object of given type to var, which returns error in
 * case of failiture
 */
#define GRAPH_ALLOCATE(type, var, error) \
	do { \
		if (NULL == (var = (type*)malloc(sizeof(type)))) { \
			return error; \
		} \
	} while(false)

/** Creates new directed edge for graph */
static GraphEdge graphEdgeCreate(Graph graph, GraphVertex from, GraphVertex to) {
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

/** Copies a directed edge  */
static GraphEdge graphEdgeCopy(GraphEdge edge) {
	return graphEdgeCreate(edge->graph, edge->from, edge->to);
}

/** Frees a directed edge */
static void graphEdgeFree(GraphEdge edge) {
	if (edge == NULL) {
		return;
	}
	edge->graph->freeVertex(edge->from);
	edge->graph->freeVertex(edge->to);
	free(edge);
}

/**
 * Compares two edges
 * if first from-vertex less then second returns negative number
 * if first from-vertex greater then second returns positive number
 * if from-vertices are equal returns negative number if first to-vertex less
 * then second, positive if first greater then second and zero if edges are equal
 */
static int graphEdgeCompare(GraphEdge edge1, GraphEdge edge2) {
	assert(edge1->graph == edge2->graph);
	int fromDifference = edge1->graph->compareVertex(edge1->from, edge2->from);
	if (0 == fromDifference) {
		return edge1->graph->compareVertex(edge1->to, edge2->to);
	}
	return fromDifference;
}

Graph graphCreate(copyGraphVertex copyVertex, compareGraphVertex compareVertex, freeGraphVertex freeVertex) {
	if (!copyVertex || !compareVertex || !freeVertex) {
		return NULL;
	}
	Graph graph;
	GRAPH_ALLOCATE(Graph_t, graph, NULL);

	graph->copyVertex = copyVertex;
	graph->compareVertex = compareVertex;
	graph->freeVertex = freeVertex;
	graph->vertices = setCreate(copyVertex, freeVertex, compareVertex);
	graph->edges = setCreate((copySetElements)graphEdgeCopy, (freeSetElements)graphEdgeFree, (compareSetElements)graphEdgeCompare);

	return graph;
}

void graphDestroy(Graph graph) {
	if (graph == NULL) {
		return;
	}
	setDestroy(graph->vertices);
	setDestroy(graph->edges);
	free(graph);
}

GraphResult graphAddVertex(Graph graph, GraphVertex vertex) {
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

GraphResult graphRemoveVertex(Graph graph, GraphVertex vertex){
	if (graph == NULL || vertex == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (graphIsVertexExists(graph, vertex) == false) {
		return GRAPH_VERTEX_DOES_NOT_EXISTS;
	}

	// remove vertex
	SetResult removing = setRemove(graph->vertices, vertex);
	assert(removing == SET_SUCCESS);

	// remove all incident edges
	for (bool found = true; found; ) {
		found = false;
		SET_FOREACH(GraphEdge, edge, graph->edges) {
			if (graph->compareVertex(edge->from, vertex) == 0 ||
					graph->compareVertex(edge->to, vertex) == 0 ) {
				SetResult removeResult = setRemove(graph->edges, edge);
				assert(removeResult == SET_SUCCESS);
				found = true;
				break;
			}
		}
	}
	return GRAPH_SUCCESS;
}

bool graphIsVertexExists(ConstGraph graph, GraphVertex vertex) {
	if (graph == NULL || vertex == NULL) {
		return false;
	}

	return setIsIn(graph->vertices, vertex);
}

GraphResult graphAddDirectedEdge(Graph graph, GraphVertex from, GraphVertex to){
	if (graph == NULL || from == NULL || to == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (!graphIsVertexExists(graph, from) || !graphIsVertexExists(graph, to)) {
		return GRAPH_VERTEX_DOES_NOT_EXISTS;
	}
	if (graphIsDirectedEdgeExists(graph, from, to)) {
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
	graphEdgeFree(newEdge);
	return GRAPH_SUCCESS;
}

GraphResult graphRemoveDirectedEdge(Graph graph, GraphVertex from, GraphVertex to) {
	if (graph == NULL || from == NULL || to == NULL) {
		return GRAPH_NULL_ARGUMENT;
	}
	if (!graphIsDirectedEdgeExists(graph, from, to)) {
		return GRAPH_EDGE_DOES_NOT_EXISTS;
	}
	GraphEdge edge = graphEdgeCreate(graph, from, to);
	if (edge == NULL) {
		return GRAPH_OUT_OF_MEMORY;
	}
	SetResult setRemoveResult = setRemove(graph->edges, edge);
	assert(setRemoveResult == SET_SUCCESS);
	graphEdgeFree(edge);
	return GRAPH_SUCCESS;
}

bool graphIsDirectedEdgeExists(Graph graph,  GraphVertex from, GraphVertex to){
	if (graph == NULL || from == NULL || to == NULL) {
		return false;
	}

	GraphEdge edge = graphEdgeCreate(graph, from, to);
	bool isIn = setIsIn(graph->edges, edge);
	graphEdgeFree(edge);
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
