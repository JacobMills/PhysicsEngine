#ifndef COLLISIONH
#define COLLISIONH

// Includes //
#include "core.h"
#include "vertex.h"
#include "body.h"
#include "vector2.h"
#include "shape.h"

// Constants //
//in final physics engine each object could have its own co-efficient of restitution. 
const float restitution = 1/*0.4f*/;

// Contact Generation //

// Contact //
class Contact
{
private: 
	//information about the contact
	Vector2 contactPoint;
	Vector2 contactNormal;
	float penetration;

	//the bodies involved
	Body* body[2] ; 


public:
	Contact():contactPoint(-1,-1){};
	//accessors
	void SetContactPoint(Vector2 newPoint) { contactPoint = newPoint; }
	void SetContactNormal(Vector2 newNormal) { contactNormal = newNormal; }
	void SetPenetration(float newPenetration) { penetration = newPenetration; }
	void SetBodyData( Body *newBody1, Body *newBody2) { body[0] = newBody1; body[1] = newBody2; }

	Body* GetBody(const unsigned index) const {if (index > 1) return new Body(Vector2(-1,-1));	return body[index];}
	Vector2 GetContactPoint() const { return contactPoint;}
	Vector2 GetContactNormal() const { return contactNormal; }

	// Resolves position of the bodies in contact
	void ResolvePosition()
	{
		//Linear inertia for the two bodies
		float linearInertia[2] = {0,0};

		//inverse of total inertia in the collision
		float inverseInertia = 0;

		//for each body, if there is one (halfspaces input NULL as they will never move)
		for (unsigned i=0; i<2; i++)
		{
			if (body[i])
			{
				//get the inverse mass for movement amount, add to inverse inertia
				linearInertia[i] = body[i]->inverseMass;

				//add to inverse total inertia
				inverseInertia += linearInertia[i];
			}
		}

		//invert it
		inverseInertia = 1/inverseInertia;

		//calculate amount to move based on above values
		float linearMove[2] = {penetration * linearInertia[0] * inverseInertia, -penetration * linearInertia[1] * inverseInertia};

		//if body is present, move it
		for (unsigned i=0; i<2; i++)
		{
			if (body[i])
			{
				body[i]->position += contactNormal*linearMove[i];
			}
		}
	}

	//TODO: Thought doing velocity resolution with rotation would enlighten me on what's wrong with this.
	//There's plenty wrong with this, but velocity resolution doesn't seem to work either, so I'm not gonna bother.
	//resolves position of bodies in contact... trying to do it nonlinearly
	void ResolvePositionWithRotation()
	{	
		//Linear inertia for the two bodies
		float linearInertia[2] = {0,0};

		//inverse of total inertia in the collision
		float inverseInertia = 0;
		
		//angular inertia for the two bodies
		float angularInertia[2] = {0,0};


		//for each body, if there is one (halfspaces input NULL as they will never move)
		for (unsigned i=0; i<2; i++)
		{
			if (body[i])
			{
				//get the inverse mass for movement amount, add to inverse inertia
				linearInertia[i] = body[i]->inverseMass;

				//get angular inertia by transforming contact point to local coordinates for the body,
				//then finding some weird pseudo-torque using the contact normal
				Vector2 translation = body[i]->position.GetInvert();
				float rotation = -body[i]->orientation;

				Vector2 localContactPosition = contactPoint + translation;
				localContactPosition.RotateAboutWorldOrigin(rotation);

				Vector2 localContactNormal = contactNormal;
				localContactNormal.RotateAboutWorldOrigin(rotation);

				angularInertia[i] = CalculateLocalTorque(localContactPosition, localContactNormal); 
				angularInertia[i] *= body[i]->inverseMomentOfInertia; //not right? 1/I*torque = angular acceleration, not motion? we want a change in angular motion?
				

				//add to inverse total inertia
				inverseInertia += linearInertia[i] + angularInertia[i];
			}
		}
		
		//invert it
		inverseInertia = 1/inverseInertia;

		//calculate amount to move based on above values

		float linearMove[2] = {penetration * linearInertia[0] * inverseInertia, -penetration * linearInertia[1] * inverseInertia};

			//this number is meant to be the distance it needs to rotate, not the rotation? is it? -> how to turn this into #of degrees?
				//-> i have the body's position and the relative position of the contact - must be able to get amount of rotation around that point to get a certain distance?
				//-> doesn't matter because this value still looks wrong.
				//-> to sort this out, would need to figure out the impulse to move it the amount.
		float angularMove[2] = {penetration * angularInertia[0] * inverseInertia, -penetration * angularInertia[1] * inverseInertia};
		
		//if body is present, move it
		for (unsigned i=0; i<2; i++)
		{
			if (body[i])
			{
				body[i]->position += contactNormal*linearMove[i];
				//body[i]->orientation += angularInertia[i];
				body[i]->orientation += angularMove[i]; //probably definitely not right, just thrown in for fun
			}
		}

	}

	//resolves the velocities (linear and angular) of two intersecting bodies
	//TODO: rotation is a nightmare, I don't know. figuring out rotational change in velocity per unit impulse might be wrong, or the way 
	//I'm applying the change at the end could be. Rotation seems to steal most of the linear velocity change anyway. 
	void ResolveVelocitiesAndRotations()
	{
		//figuring out an impulse that changes the linear velocity causing the intersection to go away and rotate the body accordingly

		//firstly want to find the velocity change per impulse applied for the contacts - linear and angular
			//linear change in velocity for a unit impulse is in direction of impulse, magnitude of which is inverse mass

		float linearChangeInVelocityPerUnitImpulse = 0;
		Vector2 relativeContact[2] ;

		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				linearChangeInVelocityPerUnitImpulse += body[i]->inverseMass;

				//get relative contact position
				relativeContact[i] = contactPoint - body[i]->position;
				relativeContact[i].RotateAboutWorldOrigin(-body[i]->orientation);
			}
		}

		
		//angular change is difficult.
		float rotationalChangeInVelocityPerUnitImpulse = 0;
		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{		
				Vector2 relativeContactNormal = contactNormal;
				relativeContactNormal.RotateAboutWorldOrigin(-body[i]->orientation);

					//get impulsive torque generated by one unit's impulse (contact normal)
				float impulsiveTorquePerUnitImpulse = CalculateLocalTorque(relativeContact[i], relativeContactNormal); 

					//get change in angular velocity that you get from a unit of impuslive torque
				float changeInAngularVelocityPerUnitImpulsiveTorque = impulsiveTorquePerUnitImpulse * body[i]->inverseMomentOfInertia;

					//get linear velocity of point due to rotation only
					//Velocity of point on rotating body: vx = -ry*w, vy = rx*w (http://www.euclideanspace.com/physics/kinematics/combinedVelocity/index.htm)
				Vector2 velocityOfPointDueToRotation(-relativeContact[i].y * changeInAngularVelocityPerUnitImpulsiveTorque, relativeContact[i].x * changeInAngularVelocityPerUnitImpulsiveTorque);
		

				//only interested in magnitude relative to contact normal 
				//finding proportion of vector relative to other vector: a.b = |a||b|cos(theta), and |a|cos(theta) = |projection of a onto b|, so a.b/|b| = |projection of a onto b|
				//(Khan Academy http://youtu.be/KDHuWxy53uM?t=4m12s)
				float projection =  relativeContactNormal * velocityOfPointDueToRotation; 
				projection /= velocityOfPointDueToRotation.Magnitude();
				
				rotationalChangeInVelocityPerUnitImpulse += (velocityOfPointDueToRotation * projection).Magnitude();
			}
		}

		float velocityChangePerUnitImpulse = linearChangeInVelocityPerUnitImpulse + rotationalChangeInVelocityPerUnitImpulse;



		//need closing velocity at contact point, from linear and rotational movement
			//linear movement is stored in the body
		Vector2 linearClosingVelocity(0,0);

		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				if (i == 0)
					linearClosingVelocity += body[i]->velocity;//body[1] needs to take away instead of add so negative velocities add to closing velocity
				else 
					linearClosingVelocity -= body[i]->velocity;
			}
		}
			//need to know how much is in direction of contact normal (and how much is at a tangent to it for friction)
		float linearClosingVelocityProjection =  contactNormal * linearClosingVelocity ; 
		linearClosingVelocityProjection /= linearClosingVelocity.Magnitude();
		
		linearClosingVelocity *= linearClosingVelocityProjection;

			//finding closing velocity due to rotation...
			//Velocity of point on rotating body: vx = -ry*w, vy = rx*w 
		Vector2 rotationalClosingVelocity(0,0);
		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				Vector2 linearVelocityFromRotation;
				linearVelocityFromRotation.x = -relativeContact[i].y * DegreesToRadians(body[i]->rotation);
				linearVelocityFromRotation.y = relativeContact[i].x * DegreesToRadians(body[i]->rotation);


				if (i == 0)
					rotationalClosingVelocity += linearVelocityFromRotation;
				else
					rotationalClosingVelocity -= linearVelocityFromRotation;	
			}
		}
		//TODO: could skip getting them separately and get them both at the same time using velocity.x-ry*w and velocity.y+rx*w

		//how much is in direciton of contact normal?
		float rotationClosingVelocityProjection = contactNormal * rotationalClosingVelocity ;
		rotationClosingVelocityProjection /= rotationalClosingVelocity.Magnitude();

		rotationalClosingVelocity *= rotationClosingVelocityProjection;

		float contactVelocity = linearClosingVelocity.Magnitude() + rotationalClosingVelocity.Magnitude();
		
		//change velocity: change in velocity = -(1+restitution coefficient)* closing velocity
		float deltaVelocity = -contactVelocity * (1+restitution); //might be right so far


		//impulse needed to achieve a given velocity = velocity/velocity change per unit impulse
		float impulseMag = deltaVelocity / velocityChangePerUnitImpulse; //problem with velocityChangePerUnitImpulse?
		Vector2 impulse = contactNormal.GetInvert() * impulseMag;
		
		//I THINK THE PROBLEM IS HERE: rotationalChangeInVelocityPerImpulse is about as big as the linear one, yet deltaVelocity is only .1 bigger
		//than if it was just the linear velocity resolution - so the impulse becomes much smaller, and the linear velocity produced from it just diminishes.

		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				if (i == 1)
					impulse.Invert(); 

				Vector2 velocityChange = impulse * body[i]->inverseMass;

				float impulsiveTorque = CalculateLocalTorque(relativeContact[i], impulse);
				float rotationChange = impulsiveTorque*body[i]->inverseMomentOfInertia;
				//rotationChange = RadiansToDegrees(rotationChange); //TODO: big error? rotations are way too big if in degrees? 

				body[i]->velocity += velocityChange;
				body[i]->rotation += rotationChange;

			}
		}
		
 	}


	void ResolveVelocities()
	{
		//figuring out an impulse that changes the linear velocity causing the intersection to go away accordingly

		//firstly want to find the velocity change per impulse applied for the contacts - linear and angular
		//linear change in velocity for a unit impulse is in direction of impulse, magnitude of which is inverse mass

		float linearChangeInVelocityPerUnitImpulse = 0;

		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				linearChangeInVelocityPerUnitImpulse += body[i]->inverseMass;
			}
		}

		//need closing velocity at contact point, from linear and rotational movement
		//linear movement is stored in the body
		Vector2 linearClosingVelocity(0,0);

		for (unsigned i = 0; i<2; i++)
		{
			if (body[i])
			{
				if (i == 0)
					linearClosingVelocity += body[i]->velocity;//if body[1] should this be taking away instead of adding?
				else 
					linearClosingVelocity -= body[i]->velocity;
			}
		}
		//need to know how much is in direction of contact normal and how much is at a tangent to it
		float linearClosingVelocityProjection =  contactNormal * linearClosingVelocity ;
		linearClosingVelocityProjection /= linearClosingVelocity.Magnitude();

		linearClosingVelocity *= linearClosingVelocityProjection;

		//change velocity: change in velocity = -(1+restitution coefficient)* closing velocity
		float deltaVelocity = -linearClosingVelocity.Magnitude() * (1+restitution); //might be right so far


		//impulse needed to achieve a given velocity = velocity/velocity change per unit impulse
		float impulseMag = deltaVelocity / linearChangeInVelocityPerUnitImpulse;
		Vector2 impulse = contactNormal.GetInvert() * impulseMag;

		Vector2 velocityChange = impulse * body[0]->inverseMass;
		//&& rotation

		body[0]->velocity += velocityChange;

		if (body[1])
		{
			velocityChange = impulse.GetInvert() * body[1]->inverseMass;
			body[1]->velocity += velocityChange;
		}
		
	}


	//outputs a string, for debugging
	std::string GetContactInfoText() const
	{
		std::string text;
		if (contactPoint.x != -1)
		{
			text += "Contact normal: ";
			text+= contactNormal.ToString();
			text+="\nContact point: ";
			text+= contactPoint.ToString();
			text+="\nPenetration depth: ";
			text+=ToString(penetration);
			text+="\n\n";
		}
		return text;
	}

};

// Object List //
class ObjectList
{
private:
	std::vector<Box*> boxes;
	std::vector<Circle*> circles;
	std::vector<HalfSpace*> halfSpaces;

public:
	// Add Items
	void Add( Box &box) { boxes.push_back(&box); }
	void Add( Circle &circle) { circles.push_back(&circle); }
	void Add( HalfSpace &halfSpace) { halfSpaces.push_back(&halfSpace); }

	void Remove(Box &box)
	{
		for (unsigned i = 0; i<boxes.size(); i++)
		{
			if (boxes[i]->IsTheSameAs(box))
			{
				boxes.erase(boxes.begin() + i);
			}
		}
	}

	void Remove(Circle &circle)
	{
		for (unsigned i = 0; i<circles.size(); i++)
		{
			if (circles[i]->IsTheSameAs(circle))
			{
				circles.erase(circles.begin() + i);
			}
		}
	}

	// Accessors
	Box GetBoxAt(const unsigned index) const { return *boxes[index]; }
	Circle GetCircleAt(const unsigned index) const { return *circles[index]; }
	HalfSpace GetHalfSpaceAt(const unsigned index) const {return *halfSpaces[index]; }

	// List Sizes 
	unsigned BoxesSize() const { return boxes.size(); }
	unsigned CirclesSize() const { return circles.size(); }
	unsigned HalfSpacesSize() const {return halfSpaces.size(); }
	unsigned Size() const { return boxes.size() + circles.size() + halfSpaces.size(); }

};

// Collision Detector //

class CollisionDetector
{
public:
	//holds functions for handling different types of collisions and generating their contact data

	// Circle and Circle //
	unsigned int CircleAndCircle(const Circle &one, const Circle &two, std::vector<Contact> &data)
	{

		//get circle positions
		Vector2 positionOne = one.GetPosition();
		Vector2 positionTwo = two.GetPosition();

		//get radii
		float radiusOne = one.GetRadius();
		float radiusTwo = two.GetRadius();

		//find vector between objects
		Vector2 midline = positionOne - positionTwo;

		//distance between origins
		float distance = midline.Magnitude();

		//check whether two circles are in collision - if distance between two centres is greater than
		//size of the two radii, they are not colliding. 
		if (distance <= 0.0f || distance >= radiusOne+radiusTwo)
		{
			//not in collision
			return 0;
		}

		//get contact normal by normalising midline vector
		Vector2 normal = midline*(1.0f/distance);

		//add data to the collisionData list
		//Contact* contact = data->Contacts;
		Contact contact;

		contact.SetContactNormal(normal);

		//contact point is on line of normal and pOne's position
		contact.SetContactPoint(positionOne-normal*radiusOne); 
		contact.SetPenetration(radiusOne+radiusTwo - distance);

		//TODO: set friction and restitution
		contact.SetBodyData(one.GetBody(), two.GetBody());


		data.push_back(contact);

		return 1;
	}

	// Draw Contact //
	unsigned int CircleAndCircle(const Circle &one, const Circle &two, std::vector<Contact> &data, VertexList &vertexList)
	{
		return DrawContactNormal(CircleAndCircle(one, two, data), data, vertexList);
	}

	 // Circle and HalfSpace //
	unsigned int CircleAndHalfSpace(const Circle &circle, const HalfSpace &halfSpace, std::vector<Contact> &data)
	{
		//cache circle position
		Vector2 position = circle.GetPosition();

		//find the distance from the plane using: distance = pointPosition . halfSpaceNormal - halfSpaceOffset 
		float distance = halfSpace.GetNormal() * position - circle.GetRadius() - halfSpace.GetOffset();

		if (distance>= 0 ) 
			return 0;

		//create the contact with normal in halfSpace direction
		Contact contact;
		contact.SetContactNormal(halfSpace.GetNormal());
		contact.SetPenetration(-distance);
		contact.SetContactPoint(position - halfSpace.GetNormal() * (distance + circle.GetRadius()));
		contact.SetBodyData(circle.GetBody(), NULL);

		data.push_back(contact);
		return 1;

	}

	// Draw Contact //
	unsigned int CircleAndHalfSpace(const Circle &circle, const HalfSpace &halfSpace, std::vector<Contact> &data, VertexList &vertexList)
	{
		return DrawContactNormal(CircleAndHalfSpace(circle, halfSpace, data), data, vertexList);
	}


	// Box and HalfSpace //
	unsigned int BoxAndHalfSpace(const Box &box, const HalfSpace &halfSpace, std::vector<Contact> &data)
	{
		//TODO: early out

		//find intersection points by checking each vertex of box.
		Vector2 vertices[4];
		box.GetVertices(vertices);

		unsigned contactCount = 0;

		float smallestDistance = FLT_MAX;
		unsigned smallestDistanceIndex = 4;

		//for each vertex
		for (int i = 0; i < 4; i++)
		{
			//get distance from halfspace
			float distance = vertices[i] * halfSpace.GetNormal();

			//if negative, collision - and if its less in than any other collisions, record this instead of the other one
			if (distance <= halfSpace.GetOffset())
			{
				if (distance <= smallestDistance)
				{
					smallestDistance = distance;
					smallestDistanceIndex = i;
				}
			}

			// Cohesion
			/*//get distance of vertex from halfSpace
			float distance = vertices[i] * halfSpace.GetNormal();

			//if this distance is negative, there's a contact
			if (distance <= halfSpace.GetOffset())
			{
				//generate contact data
				Contact contact;
				contact.SetContactPoint(/ *halfSpace.GetNormal()*(distance-halfSpace.GetOffset()) +* / vertices[i]);
				contact.SetContactNormal(halfSpace.GetNormal());
				contact.SetPenetration(halfSpace.GetOffset() - distance);
				contact.SetBodyData(box.GetBody(), NULL);

				contactCount++;
				data.push_back(contact);
				
				//TODO: support cohesion- remove this
				//return contactCount;
			}*/
		}

		// No Cohesion Pick One Remix
		// if collision, index will be 0-3
		if (smallestDistanceIndex == 4)
			return 0;
		else
		{
			//generate contact data
			Contact contact;
			contact.SetContactPoint(vertices[smallestDistanceIndex]);
			contact.SetContactNormal(halfSpace.GetNormal());
			contact.SetPenetration(halfSpace.GetOffset() - smallestDistance);
			contact.SetBodyData(box.GetBody(), NULL);

			contactCount++;
			data.push_back(contact);

		
			return contactCount;
		}
		//return contactCount;	
	}

	// Draw Contact //
	unsigned int BoxAndHalfSpace(const Box &box, const HalfSpace &halfSpace, std::vector<Contact> &data, VertexList &vertexList)
	{
		return DrawContactNormal(BoxAndHalfSpace(box, halfSpace, data), data, vertexList);
	}

	// Box and Circle //
	unsigned int BoxAndCircle(const Box &box, const Circle &circle, std::vector<Contact> &data)
	{
		Vector2 relativeCentre = circle.GetPosition();

		//get circle coordinates in box's local coordinates by translating and rotating box to world origin
		Vector2 translation = box.GetPosition().GetInvert();
		float rotation = -box.GetOrientation();

		//apply transformation to both shapes, getting the circle relative the the box
		relativeCentre += translation;
		relativeCentre.RotateAboutWorldOrigin(rotation);

		//cache values
		float radius = circle.GetRadius();
		Vector2 halfSize = box.GetHalfSize();

		//if the circle is further away than the box than its radius, early out
		if(abs(relativeCentre.x) - radius > halfSize.x || abs(relativeCentre.y) - radius > halfSize.y)
		{
			return 0;
		}

		//get the closest point on the box to the circle
		Vector2 closestPoint(0,0);
		float distance;

		//x axis
		distance = relativeCentre.x;

		if (distance > halfSize.x) 
			distance = halfSize.x;

		if (distance < -halfSize.x) 
			distance = -halfSize.x;

		closestPoint.x = distance;

		//y axis
		distance = relativeCentre.y;

		if (distance > halfSize.y)
			distance = halfSize.y;

		if (distance < -halfSize.y) 
			distance = -halfSize.y;

		closestPoint.y = distance;

		//finely check to see if we're in contact
		distance = (closestPoint - relativeCentre).SquaredMagnitude();
		if (distance>radius*radius) 
			return 0;

		//transform closest point back to world coordinates
		closestPoint.RotateAboutWorldOrigin(-rotation);
		closestPoint += translation.GetInvert();

		//generate contact
		Contact contact;
		contact.SetContactNormal((closestPoint - circle.GetPosition()).GetUnit());
		contact.SetContactPoint(closestPoint);
		contact.SetPenetration(radius - sqrt(distance));
		contact.SetBodyData(box.GetBody(), circle.GetBody());

		data.push_back(contact);

		return 1;
	}

	// Draw Contact //
	unsigned int BoxAndCircle(const Box &box, const Circle &circle, std::vector<Contact> &data, VertexList &vertexList)
	{
		return DrawContactNormal(BoxAndCircle(box, circle, data), data, vertexList);
	}

	// Box and Box //
	unsigned int BoxAndBox(const Box &box1, const Box &box2, std::vector<Contact> &data)
	{
		//axes to be checked
		Vector2 axes[4];

		//for getting the smallest overlap
		float bestOverlap = FLT_MAX;
		unsigned int bestCase;

		//distance between box centres
		Vector2 toCentre = box2.GetPosition() - box1.GetPosition();

		//for each axis
		for (unsigned int i = 0; i<4; i++)
		{
			//get the axes to be checked only when needed, to save time if first axis fails
			switch (i)
			{
			case 0:
				axes[0] = box1.GetXAxis();
				break;
			case 1:
				axes[1] = box1.GetYAxis();
				break;
			case 2:
				axes[2] = box2.GetXAxis();
				break;
			case 3:
				axes[3] = box2.GetYAxis();
				break;
			}
			
			//get overlap of projections on this axis
			float overlap = PenetrationOnAxis(box1, box2, axes[i], toCentre);

			//if 0, no overlap, no collision, return
			if (overlap < 0)
				return 0;

			//if overlap smaller than the best, it is the new best
			if (overlap < bestOverlap)
			{
				bestOverlap = overlap;
				bestCase = i;
			}
		}	

		Contact contact;
		//if the intersection is a vertex of box2 on box1's side
		if (bestCase < 2)
		{
			GenerateBoxBoxContact(box1, box2, axes[bestCase], toCentre, bestOverlap, contact );
		}
		else //else if its box2's side
		{
			GenerateBoxBoxContact(box2, box1, axes[bestCase], toCentre.GetInvert(), bestOverlap, contact );
		}

		data.push_back(contact);
		return 1;
	}
	
	// Draw Contact //
	unsigned int BoxAndBox(const Box &box1, const Box &box2, std::vector<Contact> &data, VertexList &vertexList)
	{
		return DrawContactNormal(BoxAndBox(box1, box2, data), data, vertexList);
	}


	// Checks all objects in object list against each other for collisions - obviously not optimised at all
	// returns number of collisions
	unsigned GenerateContacts(ObjectList &objects, std::vector<Contact> &contacts)
	{
		unsigned count = 0;

		//for each box
		for (unsigned box = 0; box<objects.BoxesSize(); box++)
		{
			//for each other box (ensuring not to check boxes that have already checked this one)
			for (unsigned otherBox = box+1; otherBox < objects.BoxesSize() ; otherBox++)
			{
				count+=BoxAndBox(objects.GetBoxAt(box), objects.GetBoxAt(otherBox), contacts);
			}

			//for each circle
			for (unsigned circle = 0; circle < objects.CirclesSize(); circle++)
			{
				count+=BoxAndCircle(objects.GetBoxAt(box), objects.GetCircleAt(circle), contacts);
			}

			//for each halfspace
			for (unsigned halfSpace = 0; halfSpace < objects.HalfSpacesSize(); halfSpace++)
			{
				count+=BoxAndHalfSpace(objects.GetBoxAt(box), objects.GetHalfSpaceAt(halfSpace), contacts);
			}
		}

		//for each circle
		for (unsigned circle = 0; circle<objects.CirclesSize(); circle++)
		{
			//for each other circle
			for (unsigned otherCircle = circle+1; otherCircle < objects.CirclesSize() ; otherCircle++)
			{
				count+=CircleAndCircle(objects.GetCircleAt(circle), objects.GetCircleAt(otherCircle), contacts);
			}

			//for each halfspace
			for (unsigned halfSpace = 0; halfSpace < objects.HalfSpacesSize(); halfSpace++)
			{
				count+=CircleAndHalfSpace(objects.GetCircleAt(circle), objects.GetHalfSpaceAt(halfSpace), contacts);
			}
		}

		return count;
	}

	unsigned GenerateContactsAndDraw(ObjectList &objects, std::vector<Contact> &contacts, VertexList &vertexList ) 
	{
		unsigned count = 0;

		//for each box
		for (unsigned box = 0; box<objects.BoxesSize(); box++)
		{
			//for each other box (ensuring not to check boxes that have already checked this one)
			for (unsigned otherBox = box+1; otherBox < objects.BoxesSize() ; otherBox++)
			{
				count+=BoxAndBox(objects.GetBoxAt(box), objects.GetBoxAt(otherBox), contacts, vertexList);
			}

			//for each circle
			for (unsigned circle = 0; circle < objects.CirclesSize(); circle++)
			{
				count+=BoxAndCircle(objects.GetBoxAt(box), objects.GetCircleAt(circle), contacts, vertexList);
			}

			//for each halfspace
			for (unsigned halfSpace = 0; halfSpace < objects.HalfSpacesSize(); halfSpace++)
			{
				count+=BoxAndHalfSpace(objects.GetBoxAt(box), objects.GetHalfSpaceAt(halfSpace), contacts, vertexList);
			}
		}

		//for each circle
		for (unsigned circle = 0; circle<objects.CirclesSize(); circle++)
		{
			//for each other circle
			for (unsigned otherCircle = circle+1; otherCircle < objects.CirclesSize() ; otherCircle++)
			{
				count+=CircleAndCircle(objects.GetCircleAt(circle), objects.GetCircleAt(otherCircle), contacts, vertexList);
			}

			//for each halfspace
			for (unsigned halfSpace = 0; halfSpace < objects.HalfSpacesSize(); halfSpace++)
			{
				count+=CircleAndHalfSpace(objects.GetCircleAt(circle), objects.GetHalfSpaceAt(halfSpace), contacts, vertexList);
			}
		}

		return count;
	}

	// Add draw info for all contacts in a contact list
	void DrawContacts(const std::vector<Contact> &contacts, VertexList &vertexList) const
	{
		for (unsigned i = 0; i<contacts.size(); i++)
		{
			DrawContactNormal(contacts[i], vertexList);
		}
	}

private:
	// Draw Contact Point and Normal //
	void DrawContactNormal(const Contact &contact, VertexList & vertexList ) const 
	{
		//draws a contact
		DrawLine contactNormal(contact.GetContactPoint(), contact.GetContactPoint() + contact.GetContactNormal());
		contactNormal.AddDrawInfo(vertexList, RED);
	}
	
	unsigned int DrawContactNormal(const unsigned &result, std::vector<Contact> &data, VertexList &vertexList) const
	{
		//used exclusively by contact generation functions to draw contact normal. 
		//uses function's return value to see whether to bother drawing or not.
		if (result == 0)
			return 0;

		Contact &contact = data.back();
		DrawContactNormal(contact, vertexList);

		return result;
	}


	// Box and Box Methods //

	//checks if two boxes overlap on a given axis. toCentre is the distance
	//between the centres of the two boxes, passing it in means avoiding 
	//recalculation every time
	float PenetrationOnAxis(const Box &one, const Box &two, const Vector2 &axis, const Vector2 &toCentre)
	{
		//project halfsizes onto axis 
		float oneProject = TransformToAxis(one, axis);
		float twoProject = TransformToAxis(two, axis);

		//projection of centre distances on axis
		float distance = abs(toCentre*axis);

		//check for overlap; negative value means separation
		return oneProject + twoProject - distance;
	}

	//returns projection of box on axis
	float TransformToAxis(const Box &box, const Vector2 &axis)
	{
		return box.GetHalfSize().x * abs(axis*box.GetXAxis()) + box.GetHalfSize().y * abs(axis*box.GetYAxis());
	}

	//fill the given contact with the correct information, based on the input values
	void GenerateBoxBoxContact(const Box &box1, const Box &box2, const Vector2 &axis, const Vector2 &toCentre, const float &penetration, Contact &contact)
	{		
		Vector2 normal = axis;
		//which side is in contact?
		if (normal * toCentre > 0)
			normal.Invert();

		//which of two's vertices are in contact? (in box2's local+ coords)
		Vector2 vertex = box2.GetHalfSize();
		if (box2.GetXAxis() * normal < 0) vertex.x = -vertex.x;
		if (box2.GetYAxis() * normal < 0) vertex.y = -vertex.y;

		//transform vertex to world
		Vector2 translation = box2.GetPosition();
		float rotation = box2.GetOrientation();

		vertex.RotateAboutWorldOrigin(rotation);
		vertex+=translation;

		//create the contact
		contact.SetContactNormal(normal);
		contact.SetPenetration(penetration);
		contact.SetContactPoint(vertex);
		contact.SetBodyData(box1.GetBody(), box2.GetBody());
	}
};


#endif //CONTACTGENH