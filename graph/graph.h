#ifndef GRAPH_H_
#define GRAPH_H_

/**
 * Generic directed graph
 *
 * Implements a directed graph type
 *
 * The following functions are available
 * 		graphCreate 				- Creates a new empty directed graph
 * 		graphDestroy 				- Deletes an existing graph and frees all
 * 										resources
 * 		graphAddVertex 				- Adds a new vertex to the graph
 * 		graphRemoveVertex 			- Removes a vertex which matches a given
 * 										vertex
 * 		graphIsVertexExists 		- Returns weather or not an vertex exists
 * 										inside the graph
 * 		graphAddDirectedEdge 		- Adds a new directed edge between two
 * 										vertices to the graph
 * 		graphRemoveDirectedEdge 	- Removes a directed edge between two
 * 										vertices
 * 		graphIsDirectedEdgeExists	- Returns weather or not a directed edge
 * 										between two vertices exists
 * 		graphClear					- Clears the contents of the graph, frees
 * 										all the vertices and edges
 *
 */

#include <stdbool.h>

/** Type for defining the graph */
typedef struct Graph_t *Graph;
/** Type for defining the const graph */
typedef const struct Graph_t * const ConstGraph;

/** Type used for returning error codes from graph functions */
typedef enum GraphResult_t {
	GRAPH_SUCCESS,
	GRAPH_OUT_OF_MEMORY,
	GRAPH_NULL_ARGUMENT,
	GRAPH_VERTEX_ALREADY_EXISTS,
	GRAPH_VERTEX_DOES_NOT_EXISTS,
	GRAPH_EDGE_ALREADY_EXISTS,
	GRAPH_EDGE_DOES_NOT_EXISTS
} GraphResult;

/** Vertex data type */
typedef void * GraphVertex;

/** Type of function for copying a vertex */
typedef GraphVertex(*copyGraphVertex)(GraphVertex);

/**
 * Type of function for comparing a vertices.
 * This function should return:
 * 		A positive integer if the first element is greater;
 * 		0 if they're equal;
 *		A negative integer if the second element is greater.
 */
typedef int(*compareGraphVertex)(GraphVertex, GraphVertex);

/** Type of function for deallocating a vertex */
typedef void(*freeGraphVertex)(GraphVertex);

/**
 * graphCreate: Allocates a new empty graph
 *
 * @param copyVertex - Function pointer to be used for copying vertices
 * @param compareVertex - Function pointer to be used for comparing elements
 * @param freeVertex - Function pointer to be used for deallocating vertices
 * @return
 * 		NULL - if one of the parameters in NULL or allocating failed
 * 		A new Graph in case of success
 */
Graph graphCreate(copyGraphVertex copyVertex, compareGraphVertex compareVertex, freeGraphVertex freeVertex);

/**
 * graphDestroy: Deallocates an existing graph. Clears all vertices by using the
 * stored free function.
 *
 * @param graph - Target graph to be deallocated. If graph is NULL nothing will
 * 		be done
 */
void graphDestroy(Graph graph);

/**
 * graphAddVertex: Adds a new vertex to the graph
 *
 * @param graph - The graph for which to add an vertex
 * @param vertex - The vertex to insert. A copy of vertex will be inserted as
 * 		supplied by the copying function which is given at initialization
 * @return
 * 		GRAPH_NULL_ARGUMENT if one of the parameters is NULL
 * 		GRAPH_VERTEX_ALREADY_EXISTS if an equal vertex already exists
 * 		GRAPH_OUT_OF_MEMORY if an allocating failed
 * 		GRAPH_SUCCESS if the vertex was successfully added
 */
GraphResult graphAddVertex(Graph graph, GraphVertex vertex);

/**
 * graphRemoveVertex: Removes a vertex and all adjacent edges from the graph. The vertex is found
 * using the comparison function given at initialization. Once found, the element
 * is removed and deallocated using the free function.
 *
 * @param graph - The graph to remove vertex from
 * @param vertex - The vertex to remove from graph
 * @return
 * 		GRAPH_NULL_ARGUMENT if one of the parameters is NULL
 * 		GRAPH_VERTEX_DOES_NOT_EXISTS if the vertex doesn't exists in the graph
 * 		GRAPH_SUCCESS if the vertex was successfully removed
 */
GraphResult graphRemoveVertex(Graph graph, GraphVertex vertex);

/**
 * graphIsVertexExists: Checks if a vertex exists in the graph. The vertex will
 * be considered in the graph if one of the vertices if the graph is determined equal
 * using the comparison function.
 *
 * @param graph - The graph to search in
 * @param vertex - The vertex to look for.
 * @return
 * 		false if one of parameters is NULL or vertex was not found
 * 		true if the vertex was found in the graph
 */
bool graphIsVertexExists(ConstGraph graph, GraphVertex vertex);

/**
 * graphAddDirectedEdge: Adds a new directed edge between two vertices to the graph
 *
 * @param graph - The graph to which to add a directed edge
 * @param from - The vertex where directed edge begins
 * @param to - The vertex where directed edge ends
 * @return
 * 		GRAPH_NULL_ARGUMENT if one of parameters is NULL
 * 		GRAPH_VERTEX_DOES_NOT_EXISTS if one of vertices does not exists in graph
 * 		GRAPH_EDGE_ALREADY_EXISTS if a directed edge between vertices already
 * 			exists in graph
 * 		GRAPH_OUT_OF_MEMORY if an allocation failed
 * 		GRAPH_SUCCESS if a directed edge was successfully added
 */
GraphResult graphAddDirectedEdge(Graph graph, GraphVertex from, GraphVertex to);

/**
 * graphRemoveDirectedEdge: Removes an directed edge if exists.
 *
 * @param graph - The graph to remove edge from
 * @param from - The vertex where directed edge to removing starts
 * @param to - The vertex where directed edge to remove ends
 * @return
 */
GraphResult graphRemoveDirectedEdge(Graph graph, GraphVertex from, GraphVertex to);

/**
 * graphIsDirectedEdgeExists: Checks if directed edge present in graph
 *
 * @param graph - The graph to search in
 * @param from - The start of directed edge to look for
 * @param to - The end of directed edge to look for
 * @return
 * 		false if one of parameters if NULL or edge was not found
 * 		true if edge is present in graph
 */
bool graphIsDirectedEdgeExists(Graph graph,  GraphVertex from, GraphVertex to);

/**
 * graphClear: Removes all vertices and edges from target graph
 *
 * @param graph - Target graph to remove all vertices and edges from
 * @return
 * 		GRAPH_NULL_ARGUMENT if a NULL pointer was sent
 * 		GRAPH_SUCCESS - Otherwise
 */
GraphResult graphClear(Graph graph);

#endif /* GRAPH_H_ */

