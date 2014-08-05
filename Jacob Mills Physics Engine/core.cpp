#include "core.h"

//get pixels for input metres
float MetresToPixels(const float metres)
{
	return (metres*pixMRatio);
}

//get metres for input pixels (probably never used)
float PixelsToMetres(const float pixels) 
{
	return (pixels*mPixRatio);
}

float DegreesToRadians(const float degrees)
{
	return (degrees*Pi/180);
}


float RadiansToDegrees( float radians )
{
	return (radians*(180/Pi));
}


//convert float to string
std::string ToString (const float number)
{
	std::ostringstream buff;
	buff<<number;
	return buff.str();
}
