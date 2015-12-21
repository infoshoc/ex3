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
	const char * const vertex1 = "cherkasy";
	const char * const vertex2 = "kiev";
	const char * const vertex3 = "lviv";
	const char * const vertexNotInGraph = "haifa";

	Graph graph = graphCreate(stringCopy, strcmp, free);
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
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex2, vertex3));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex3, vertexNotInGraph) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex3, vertexNotInGraph));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertexNotInGraph, vertex3) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertexNotInGraph, vertex3));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_ALREADY_EXISTS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));

	//remove some
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertex2));

	graphDestroy(graph);
	return true;
}

static bool graphAddDirectedEdgeTest() {
	// vertices
	const char * const vertex1 = "Cherkasy";
	const char * const vertex2 = "Lviv";
	const char * const vertex3 = "Kiev";
	const char * const vertex4 = "Amber";
	const char * const vertexNotInGraph = "Haifa";

	Graph graph = graphCreate(stringCopy, free, strcmp);
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
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertexNotInGraph));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertexNotInGraph, vertex1) == GRAPH_VERTEX_DOES_NOT_EXISTS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertexNotInGraph, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_EDGE_ALREADY_EXISTS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex3) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex3));
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex3, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex1));


	graphDestroy(graph);
	return true;
}

static bool graphIsDirectedEdgeTest() {
	// vertices
	const char * const vertex1 = "Cherkasy";
	const char * const vertex2 = "Lviv";
	const char * const vertex3 = "Kiev";
	const char * const vertex4 = "Amber";
	const char * const vertexNotInGraph = "Haifa";

	Graph graph = graphCreate(stringCopy, free, strcmp);
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
	ASSERT_TEST(!graphIsDirectedEdge(NULL, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, NULL, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, NULL));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertexNotInGraph));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertexNotInGraph, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex1, vertex2) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphAddDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex2, vertex1));
	ASSERT_TEST(graphRemoveDirectedEdge(graph, vertex2, vertex1) == GRAPH_SUCCESS);
	ASSERT_TEST(graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex2, vertex1));

	graphClear(graph);
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex1, vertex2));
	ASSERT_TEST(!graphIsDirectedEdge(graph, vertex2, vertex1));

	graphDestroy(graph);
	return true;
}

int main() {
	RUN_TEST(graphDestroyTest);
	RUN_TEST(graphAddDirectedEdgeTest);
	RUN_TEST(graphIsDirectedEdgeTest);

	return 0;
}
