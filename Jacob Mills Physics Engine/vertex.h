#ifndef VERTEXH
#define VERTEXH

// Includes //
#ifndef VECTORH
#define VECTORH
	#include <vector>
#endif

// Vertex //
struct Vertex
{
	//x and y refer to place on screen
	//weight refers to amount of colour on vertex
	//z is redundant
	float x, y, z, weight;
	DWORD colour;
};

// Vertex List //
typedef std::vector<std::vector<Vertex>> VertexList;

#endif //VERTEXH