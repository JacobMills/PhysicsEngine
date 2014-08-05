#ifndef BODYH
#define BODYH

#include "vector2.h"

// Stores data necessary for a physical rigid body
class Body
{
private:
	//position of the body in 2d space
	Vector2 position;

	//the body's current velocity
	Vector2 velocity;

	//inverse mass of the body
	float inverseMass;

	//inverse moment of inertia for the body
	float inverseMomentOfInertia;

	//current rotation of the body, in degrees - take care not to go over 360 or under 0
	float orientation;

	//current angular velocity of the body
	float rotation;

	float GetMass() const{return 1.0f/inverseMass; }

public:
	Body(): position(Vector2(3,3)), inverseMass(0.1f), inverseMomentOfInertia(0.08f), orientation(0), rotation(1), velocity(10,0){};
	Body(const Vector2 newPos): position(newPos), inverseMass(0.1f), inverseMomentOfInertia(0.08f), orientation(0), rotation(1), velocity(10,0){};
	Body(const Vector2 newPos, const float newOrientation): position(newPos), orientation(newOrientation), inverseMass(0.1f), inverseMomentOfInertia(0.08f), rotation(1), velocity(10, 0){};
	//Body(const Vector2 newPos, const float newInvMass): position(newPos), inverseMass(newInvMass), inverseMomentOfInertia(0.08f), orientation(0){};
	

	//(moment of inertia should always be calculated according to these values anyway...?)
//	Body(const Vector2 newPos, const float newInvMass, const float newInvInertia):
//		position(newPos), inverseMass(newInvMass), inverseMomentOfInertia(newInvInertia){};

	// Make data available to collidable objects
	friend class Shape;
	friend class Line;
	friend class HalfSpace;
	friend class Circle;
	//friend class UserCircle;
	friend class Box;
	//friend class UserBox;
	
	friend class Contact;
};

// Functions
float CalculateLocalTorque(const Vector2 localPoint, const Vector2 force); //TODO: move to core?

#endif