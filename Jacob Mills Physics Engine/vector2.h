#ifndef VECTOR2H
#define VECTOR2H

#include "core.h"

// Vector2 //
//Holds a 2D vector. Based on Vector class in Game Physics Engine Development, 2nd Edition, Millington
class Vector2
{
public:
	float x;	//TODO: performance improved by providing float pad[2]?
	float y;

	Vector2(): x(0), y(0){}	//regular constructor is a vector at (0,0)
	Vector2(const float newX, const float newY): x(newX), y(newY){}	//sets a vector with input values

	//flips vector in opposite direction
	void Invert()
	{
		x = -x;
		y = -y;
	}

	//returns negative vector
	Vector2 GetInvert() const
	{
		return (Vector2(-x, -y));
	}

	//return magnitude of this vector
	float Magnitude() const
	{
		return sqrtf(x*x + y*y);
	}

	//returns squared magnitude of vector - faster than magnitude, for size comparisons
	float SquaredMagnitude() const
	{
		return x*x + y*y;
	}

	//changes vector to unit vector 
	void Normalise()
	{
		float mag = Magnitude();
		if (mag > 0)
		{
			(*this) *= 1/mag;
		}
	}

	//return normalised vector
	Vector2 GetUnit() const
	{
		float mag = Magnitude();
		Vector2 unit(*this);
		if (mag>0)
		{
			unit *= 1/mag;
		}

		return unit;
	}

	//multiplies vector by given scalar
	void operator*=(const float value)
	{
		x *= value;
		y *= value;
	}

	//returns vector given by multiplying with scalar
	Vector2 operator*(const float value) const
	{
		return Vector2(x*value, y*value);
	}

	//adds vector to this
	void operator+=(const Vector2& v)
	{
		x += v.x;
		y += v.y;
	}

	//returns two added vectors
	Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x+v.x, y+v.y);
	}

	//subtracts vector from this
	void operator-=(const Vector2& v)
	{
		x -= v.x;
		y -= v.y;
	}

	//returns vector subtracted from this
	Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x-v.x, y-v.y);
	}

	//adds vector to this, and scales it
	Vector2 AddScaledVector(const Vector2& v, float scale)
	{
		x += v.x*scale;
		y += v.y*scale;
	}

	//return dot product of this and another vector
	float operator*(const Vector2 &v) const
	{
		return x*v.x + y*v.y;
	}

	//return vector perpendicular to this one (equivalent use to 3D cross product)
	Vector2 Perpendicular() const
	{
		return Vector2(-y, x);
	}

	//takes two vectors, makes them normalised and perpendicular (ie making a new orthonormal basis) //TODO: used?
	void MakePerpendicular(Vector2 *a, Vector2 *b)
	{
		a->Normalise();
		(*b) = a->Perpendicular();
	}

	//rotates point around a specified point and rotation
	void RotateAboutPoint(const Vector2 point, const float rotation)
	{
		//translate to world origin relative to point given
		Vector2 translation = point.GetInvert();
		(*this) += translation;

		//rotate it
		RotateAboutWorldOrigin(rotation);

		(*this) += translation.GetInvert();
	}

	//rotates point around world origin
	void RotateAboutWorldOrigin(const float rotation)
	{
		//prepare data
		float tempX = x;
		float tempY = y;
		float cosTheta = cos(DegreesToRadians(rotation));
		float sinTheta = sin(DegreesToRadians(rotation));

		//x' = xcos@ - ysin@  - @ is totally a theta
		x = tempX * cosTheta - tempY * sinTheta;

		//y' = xsin@ + ycos@
		y = tempX * sinTheta + tempY * cosTheta;
	}

	//convert Vector2 to string format: (x,y)
	std::string ToString() const
	{
		std::ostringstream buff;
		buff<<"(";
		buff<<x;
		buff<<",";
		buff<<y;
		buff<<")";
		return buff.str();
	}

};

#endif //VECTOR2H