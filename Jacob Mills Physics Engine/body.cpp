#include "body.h"

// Calculate a torque for a body from a point (relative to the body) and a force
float CalculateLocalTorque( const Vector2 localPoint, const Vector2 force )
{
	//Derived from torque in 3D, wich uses cross product: torque = pxfy - pyfx
	return localPoint.x*force.y - localPoint.y*force.x;
}
