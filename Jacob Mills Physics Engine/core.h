#ifndef COREH
#define COREH

// Defines // 
#define Device LPDIRECT3DDEVICE9 
#define Pi D3DX_PI
#define VertexBuffer LPDIRECT3DVERTEXBUFFER9
#define InputDevice LPDIRECTINPUTDEVICE8
#define WHITE 0xffffffff
#define YELLOW 0xffffff00
#define RED	0xffff0000
#define GREEN 0xff00ff00
#define PINK 0xffff3e96
#define ORANGE 0xffffa54f

// Includes //
#ifndef DIRECT3DH
#define DIRECT3DH
	#include <d3d9.h>
	#include <d3dx9.h>
#endif

#ifndef MATHH
#define MATHH
	#include <math.h>
#endif

#ifndef VECTORH
#define VECTORH
	#include <vector>
#endif

#include <sstream>

// Constants //
const int pixMRatio = 20; //ratio of pixels to metres
const float mPixRatio = 1.0f/pixMRatio; //ratio of metres to pixels 

enum ObjectType{BOX, CIRCLE, HALFSPACE, SHAPE};

// Functions //
// Conversions
float MetresToPixels(float metres); //get pixels for input metres
float PixelsToMetres(float pixels); //get metres for input pixels 
float DegreesToRadians(float degrees);		//get radians for input degrees
float RadiansToDegrees(float radians);

// String conversion
std::string ToString (float number);

#endif //COREH