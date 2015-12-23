#include "test_utilities.h"
#include "../graph.h"
#include <stdlib.h>
#include <string.h>

static char *stringCopy(char *string) {
	char *copy = (char*)malloc(strlen(string)+1);
	if (!copy) {
		return NULL;
	}
	strcpy(copy, string);
	return copy;
}

static bool graphDestroyTest() {
	// is NULL stable
	graphDestroy(NULL);

	// try to provoke memory leak

	// vertices
	char * vertex1 = "cherkasy";
	char * vertex2 = "kiev";
	char * vertex3 = "lviv";
	char * vertexNotInGraph = "haifa";

	Graph graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, free);
	if (!graph) {
		return false;
	}

	// add vertices
	ASSERT_TEST(graphAddVertex(graph, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(graphAddVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphAddVertex(graph, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex3));
	ASSERT_TEST(graphAddVertex(graph, vertex3) == GRAPH_VERTEX_ALREADY_EXISTS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex3));
	ASSERT_TEST(!graphIsVertexExists(graph, vertexNotInGraph));

	//add edges
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex3));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex3, vertexNotInGraph) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex3, vertexNotInGraph));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertexNotInGraph, vertex3) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertexNotInGraph, vertex3));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_ALREADY_EXISTS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));

	//remove some
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));

	graphDestroy(graph);
	return true;
}

static bool graphAddDirectedEdgeTest() {
	// vertices
	char * vertex1 = "Cherkasy";
	char * vertex2 = "Lviv";
	char * vertex3 = "Kiev";
	char * vertex4 = "Amber";
	char * vertexNotInGraph = "Haifa";

	Graph graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, (freeGraphVertex)free);
	ASSERT_TEST(graph != NULL);

	// vertex adding
	ASSERT_TEST(graphAddVertex(graph, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex4) == GRAPH_SUCCESS);

	// check
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphIsVertexExists(graph, vertex3));
	ASSERT_TEST(graphIsVertexExists(graph, vertex4));
	ASSERT_TEST(!graphIsVertexExists(graph, vertexNotInGraph));

	// edges
	ASSERT_TEST(graphAddDirectedEdge(NULL, vertex1, vertex2) == GRAPH_NULL_ARGUMENT);
	ASSERT_TEST(graphAddDirectedEdge(graph, NULL, vertex2) == GRAPH_NULL_ARGUMENT);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, NULL) == GRAPH_NULL_ARGUMENT);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertexNotInGraph) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertexNotInGraph));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertexNotInGraph, vertex1) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertexNotInGraph, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_ALREADY_EXISTS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_ALREADY_EXISTS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex3));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex3, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex1));


	graphDestroy(graph);
	return true;
}

static bool graphIsDirectedEdgeTest() {
	// vertices
	char * vertex1 = "Cherkasy";
	char * vertex2 = "Lviv";
	char * vertex3 = "Kiev";
	char * vertex4 = "Amber";
	char * vertexNotInGraph = "Haifa";

	Graph graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, (freeGraphVertex)free);
	ASSERT_TEST(graph != NULL);

	// vertex adding
	ASSERT_TEST(graphAddVertex(graph, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex4) == GRAPH_SUCCESS);

	// check
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphIsVertexExists(graph, vertex3));
	ASSERT_TEST(graphIsVertexExists(graph, vertex4));
	ASSERT_TEST(!graphIsVertexExists(graph, vertexNotInGraph));

	// edges
	ASSERT_TEST(!graphIsDirectedEdgeExists(NULL, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, NULL, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, NULL));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertexNotInGraph));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertexNotInGraph, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));

	graphClear(graph);
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));

	graphDestroy(graph);
	return true;
}

static bool graphRemoveVertexTest() {
	// vertices
	char * vertex1 = "Cherkasy";
	char * vertex2 = "Lviv";
	char * vertex3 = "Amber";
	char * vertex4 = "Haifa";
	char * vertex5 = "TelAviv";

	Graph graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, free);
	ASSERT_TEST(graph != NULL);

	ASSERT_TEST(graphAddVertex(graph, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex4) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddVertex(graph, vertex5) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphIsVertexExists(graph, vertex4));
	ASSERT_TEST(graphIsVertexExists(graph, vertex5));

	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex4, vertex5) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex4, vertex5));

	// is NULL stable
	ASSERT_TEST(graphRemoveVertex(NULL, NULL) == GRAPH_NULL_ARGUMENT);
	ASSERT_TEST(graphRemoveVertex(graph, NULL) == GRAPH_NULL_ARGUMENT);
	ASSERT_TEST(graphRemoveVertex(NULL, vertex1) == GRAPH_NULL_ARGUMENT);


	ASSERT_TEST(graphRemoveVertex(graph, vertex3) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(graphRemoveVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(!graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphIsVertexExists(graph, vertex4));
	ASSERT_TEST(graphIsVertexExists(graph, vertex5));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex4, vertex5));
	ASSERT_TEST(graphAddVertex(graph, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsVertexExists(graph, vertex1));
	ASSERT_TEST(graphIsVertexExists(graph, vertex2));
	ASSERT_TEST(graphIsVertexExists(graph, vertex4));
	ASSERT_TEST(graphIsVertexExists(graph, vertex5));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(!graphIsDirectedEdgeExists(graph, vertex2, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex4, vertex5));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex1));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex2, vertex2));
	ASSERT_TEST(graphIsDirectedEdgeExists(graph, vertex4, vertex5));

	graphDestroy(graph);

	return true;
}

static bool graphCreateTest() {
	Graph graph;
	graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, free);
	ASSERT_TEST(graph != NULL);
	graphDestroy (graph);
	graph = graphCreate(NULL, (compareGraphVertex)strcmp, free);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate((copyGraphVertex)stringCopy, NULL, free);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, NULL);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate(NULL, NULL, free);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate(NULL, (compareGraphVertex)strcmp, NULL);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate((copyGraphVertex)stringCopy, NULL, NULL);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	graph = graphCreate(NULL, NULL, NULL);
	ASSERT_TEST(graph == NULL);
	graphDestroy (graph);
	return true;
}

static bool graphIsVertexExistsTest() {
	Graph graph;

	char * vertex = "Cherkasy";
	char * vertex5 = "Lviv";

	Graph graph = graphCreate((copyGraphVertex)stringCopy, (compareGraphVertex)strcmp, free);
	ASSERT_TEST(graph != NULL);

	ASSERT_TEST(graphAddVertex(graph, vertex) == GRAPH_SUCCESS);

	ASSERT_TEST(graphIsVertexExists(graph, vertex) == true);
	ASSERT_TEST(graphIsVertexExists(graph, NULL) == false);
	ASSERT_TEST(graphIsVertexExists(NULL, vertex5)==false);
	ASSERT_TEST(graphIsVertexExists(NULL, NULL) == false);
	ASSERT_TEST(graphIsVertexExists(graph, vertex5) == false);

	return true;
}

int main() {
	RUN_TEST(graphDestroyTest);
	RUN_TEST(graphAddDirectedEdgeTest);
	RUN_TEST(graphIsDirectedEdgeTest);
	RUN_TEST(graphRemoveVertexTest);

	RUN_TEST(graphAddVertexTest);
	RUN_TEST(graphCreateTest);
	RUN_TEST(graphIsVertexExistsTest);
	RUN_TEST(graphRemoveDirectedEdgeTest);
	RUN_TEST(graphClearTest);

	return 0;
}
