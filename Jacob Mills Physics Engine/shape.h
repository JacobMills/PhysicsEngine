#ifndef SHAPEH
#define SHAPEH

#include "core.h"
#include "vector2.h"
#include "body.h"
#include "vertex.h"


// DrawLine //
class DrawLine
{
	//drawline is just like line, but without any physical information (body) stored... so just for drawing, really
	//line is stored parametrically - as an origin point and a direction vector
private:
	Vector2 origin; //position of start of line
	Vector2 direction; //vector of the direction and length of the line

public:
	//no parameters creates a line 10m long at (3,3), horizontal
	DrawLine(): origin(3,3), direction(10,0){}

	//provide a position for the origin of the line, get a 10m line, horizontal
	DrawLine(const Vector2 newPos): 
	origin(newPos), direction(newPos.x+10, newPos.y) {} 

	//provide position and length, get horizontal
	DrawLine(const Vector2 newPos, const float newLength): 
	origin(newPos), direction(newPos.x+newLength, newPos.y){}

	//provide an origin and a direction, get that line
	DrawLine(const Vector2 newOrigin, const Vector2 newDirection):
	origin(newOrigin),  direction(newDirection) {}

	//provide position, size and rotation
	DrawLine (const Vector2 newPos, const float newLength, const float newRot)
	{
		(*this) = DrawLine(newPos, newLength);

		RotateAboutLineOrigin(newRot);
	}

	//draw the line (not really part of the physics engine)
	void AddDrawInfo(VertexList &vertexList, DWORD colour) const
	{
		std::vector<Vertex> vertices;

		Vertex vertex = {MetresToPixels(origin.x),MetresToPixels(origin.y), 0, 1, colour};
		vertices.push_back(vertex);

		//get vertex of other end
		vertex.x = MetresToPixels(direction.x);
		vertex.y = MetresToPixels(direction.y);

		//add to vertices vector
		vertices.push_back(vertex);	

		//add the vertices to the vertex buffer vector
		vertexList.push_back(vertices);	
	}

	//if no colour given, just do white
	void AddDrawInfo(VertexList &vertexList) const
	{
		AddDrawInfo(vertexList, WHITE);
	}

	//draws normal of the line, at the origin point
	void DrawNormal(VertexList &vertexList) const
	{
		std::vector<Vertex> vertices;
		Vertex vertex = {MetresToPixels(origin.x), MetresToPixels(origin.y), 0, 1, YELLOW};
		vertices.push_back(vertex);

		vertex.x = MetresToPixels(origin.x-Normal().x);
		vertex.y = MetresToPixels(origin.y - Normal().y);
		vertices.push_back(vertex);

		vertexList.push_back(vertices);
	}

	//change direction vector according to rotation amount (degrees)
	void RotateAboutLineOrigin(const float rotation)
	{
		direction.RotateAboutPoint(origin, rotation);

	}

	//rotate a line about its centre-point (degrees)
	void Rotate(const float rotation)
	{
		//translate the line so that the centre point is on the world origin
		Vector2 translation = origin.GetInvert() + (origin-direction)*0.5;
		Translate(translation);

		//rotate direction, and then set origin as its inverse
		direction.RotateAboutWorldOrigin(rotation);
		origin = direction.GetInvert();

		//move line back to its position
		Translate(translation.GetInvert());
	}

	//translate the line by vector given
	void Translate(const Vector2 translation)
	{
		origin += translation;
		direction += translation;
	}

	void Translate(const float x, const float y)
	{
		origin.x+=x;
		origin.y+=y;
		direction.x+=x;
		direction.y+=y;
	}

	//return normal of line
	Vector2 Normal() const
	{
		return (direction-origin).Perpendicular().GetUnit();
	}
};

//  Collision Shapes  //

// Shape //
class Shape
{
	//parent class of all shapes. d'awww.
protected:
	//the physical information tied to this shape
	Body *body;

public:
	// Constructors
	Shape(): body(new Body()){};
	Shape(const Vector2 newPos): body(new Body(newPos)){};
	Shape(const Vector2 newPos, const float newOrientation): body(new Body(newPos, newOrientation)){};
//	Shape(const Vector2 newPos, const float newInvMass): body(new Body(newPos, newInvMass)){};
//	Shape(const Vector2 newPos, const float newInvMass, const float newInvInertia): body(new Body(newPos, newInvMass, newInvInertia)){};
	//Shapeblahlbah orientation //TODO: make things use orientation
	Shape(Body* newBody): body(newBody){};

	// Methods
	//pure virtual Draw function - every body must be able to be drawn
	virtual void AddDrawInfo(VertexList &vertexList) const {};

	void DrawVelocity(VertexList &vertexList) const
	{
		DrawLine velocity(body->position, body->position+body->velocity);
		velocity.AddDrawInfo(vertexList, ORANGE);
	}

	void DrawRotation(VertexList &vertexList) const
	{
		DrawLine rotation(body->position + body->velocity.GetUnit(), (body->position + body->velocity.GetUnit()) + (body->velocity.Perpendicular().GetUnit() * body->rotation));
		rotation.AddDrawInfo(vertexList, PINK);
	}

	void DrawVelocityAndRotation(VertexList &vertexList) const
	{
		DrawVelocity(vertexList);
		DrawRotation(vertexList);
	}

	//Translate function moves body's position, can be overridden
	virtual void Translate(const Vector2 translation)
	{
		body->position += translation;
	}

	virtual void Translate(const float x, const float y)
	{
		body->position.x += x;
		body->position.y += y;
	}

	virtual void Rotate(const float rotation)
	{
		body->orientation += rotation;

		//keep it small
		if (body->orientation >=360.0f)
			body->orientation -= 360.0f;
		if (body->orientation < 0)
			body->orientation += 360.0f;
	}

	bool BodySameAs( Body* otherBody)
	{
		if (this->body == otherBody)
			return true;

		return false;
	}

	std::string GetPositionInfoText()
	{
		std::string text;

		text += "Position: " + body->position.ToString();
		text += "\nOrientation: " + ToString(body->orientation);
		text += "\n";

		return text;
	}

	std::string GetVelocityInfoText()
	{
		std::string text;

		text += "Velocity: " + body->velocity.ToString();
		text += "\nRotation: " + ToString(body->rotation);
		text += "\n";

		return text;

	}

	// Accessors
	virtual Vector2 GetPosition() const { return body->position; }
	virtual float GetOrientation() const { return body->orientation; }
	float GetInverseMass() const { return body->inverseMass; }
	Body* GetBody() const {return body; }
	virtual ObjectType GetType() const { return SHAPE ;}

	void SetVelocity(float x, float y) { body->velocity = Vector2(x, y); }
	void SetMass(float newMass) {body->inverseMass = 1/newMass ;}
	
};

//// Line //
//NEVER USED, use halfspace for physical entity, and drawline for drawing. Was used for drawing before derived DrawLine from it.
// class Line:public Shape
// {
// 	//line is stored parametrically - as an origin point and a direction vector
// private:
// 	//body's position is used as origin of line
// 	Vector2 direction; //vector of the direction and length of the line
// 
// public:
// 	//no parameters creates a line 10m long at (3,3), horizontal
// 	Line(): Shape(), direction(13,3){}
// 
// 	//provide a position for the origin of the line, get a 10m line, horizontal
// 	Line(const Vector2 newPos): 
// 	Shape(newPos), direction(newPos.x+10, newPos.y) {} 
// 
// 	//provide position and length, get horizontal
// 	Line(const Vector2 newPos, const float newLength): 
// 	Shape(newPos), direction(newPos.x+newLength, newPos.y){}
// 
// 	//provide an origin and a direction, get that line
// 	Line(const Vector2 newOrigin, const Vector2 newDirection):
// 	Shape(newOrigin),  direction(newDirection) {}
// 
// 	//provide position, size and rotation
// 	Line (const Vector2 newPos, const float newLength, const float newRot)
// 	{
// 		(*this) = Line(newPos, newLength);
// 
// 		RotateAboutLineOrigin(newRot);
// 	}
// 
// 	//draw the line (not really part of the physics engine)
// 	void AddDrawInfo(VertexList &vertexList, DWORD colour) const
// 	{
// 		std::vector<Vertex> vertices;
// 
// 		Vertex vertex = {MetresToPixels(body->position.x),MetresToPixels(body->position.y), 0, 1, colour};
// 		vertices.push_back(vertex);
// 
// 		//get vertex of other end
// 		vertex.x = MetresToPixels(direction.x);
// 		vertex.y = MetresToPixels(direction.y);
// 
// 		//add to vertices vector
// 		vertices.push_back(vertex);	
// 
// 		//add the vertices to the vertex buffer vector
// 		vertexList.push_back(vertices);	
// 	}
// 
// 	//if no colour given, just do white
// 	void AddDrawInfo(VertexList &vertexList) const
// 	{
// 		AddDrawInfo(vertexList, WHITE);
// 	}
// 
// 	//draws normal of the line, at the origin point
// 	void DrawNormal(VertexList &vertexList) const
// 	{
// 		std::vector<Vertex> vertices;
// 		Vertex vertex = {MetresToPixels(body->position.x), MetresToPixels(body->position.y), 0, 1, YELLOW};
// 		vertices.push_back(vertex);
// 
// 		vertex.x = MetresToPixels(body->position.x-Normal().x);
// 		vertex.y = MetresToPixels(body->position.y - Normal().y);
// 		vertices.push_back(vertex);
// 
// 		vertexList.push_back(vertices);
// 	}
// 
// 	//change direction vector according to rotation amount (degrees)
// 	void RotateAboutLineOrigin(const float rotation)
// 	{
// 		direction.RotateAboutPoint(body->position, rotation);
// 	}
// 
// 	//rotate a line about its centre-point (degrees)
// 	void Rotate(const float rotation)
// 	{
// 		//translate the line so that the centre point is on the world origin
// 		Vector2 translation = body->position.GetInvert() + (body->position-direction)*0.5;
// 		Translate(translation);
// 
// 		//rotate direction, and then set origin as its inverse
// 		direction.RotateAboutWorldOrigin(rotation);
// 		body->position = direction.GetInvert();
// 
// 		//move line back to its position
// 		Translate(translation.GetInvert());
// 
// 	}
// 
// 	//translate the line by vector given
// 	void Translate(const Vector2 translation)
// 	{
// 		body->position += translation;
// 		direction += translation;
// 	}
// 
// 	void Translate(const float x, const float y)
// 	{
// 		body->position.x+=x;
// 		body->position.y+=y;
// 		direction.x+=x;
// 		direction.y+=y;
// 	}
// 
// 	//return normal of line
// 	Vector2 Normal() const
// 	{
// 		return (direction-body->position).Perpendicular().GetUnit();
// 	}
// };

// Half-Space //
//the infinite space behind an infinite length line

class HalfSpace : public Shape
{
	//half-space represented by a normal vector, and an offset along that vector that it is from the origin
	//therefore negative vectors will need a negative offset
private:
	//going to use body's position as the normal for the halfSpace
	float offset;

	//HalfSpaces shouldn't move, so make translate private. Curse inheritance
	void Translate(const Vector2 translation){}
	void Translate(const float x, const float y){}

	void Rotate(const float newOrientation){}

	//use GetNormal*GetOffset instead
	Vector2 GetPosition() const { return Vector2(-1,-1); }

public:
	// Constructors
	HalfSpace(): Shape(Vector2(0,1)/*, 0*/), offset(20.0f){body->inverseMass=0; body->inverseMomentOfInertia = 0; }

	//HalfSpace is defined as a normal vector, and an offset from the origin 
	HalfSpace(Vector2 newNormal, float newOffset):
		Shape(newNormal/*, 0*/), offset(newOffset){body->position.Normalise(); body->inverseMass=0; body->inverseMomentOfInertia = 0; }

	//make a line and draw that using the half-space data
	void AddDrawInfo(VertexList &vertexList) const
	{
		//line's origin is the point along the normal specified by the offset
		Vector2 lineOrigin = body->position * offset;

		//line direction is along the normal of the normal, placed a long way away - so i have a very long line
		Vector2 lineDirection = lineOrigin+( body->position.Perpendicular().GetUnit() * 200.0f);

		//displace origin a long way away in opposite direction
		lineOrigin += lineOrigin-lineDirection;

		//make a line out of it
		DrawLine halfSpaceLine(lineOrigin, lineDirection);

		//draw it
		halfSpaceLine.AddDrawInfo(vertexList, GREEN);
	}

	//draw the halfspace's normal
	void DrawNormal(VertexList &vertexList) const
	{
		Vector2 normalOrigin = body->position*offset;
		Vector2 normalDirection = body->position*offset - body->position;
		DrawLine normalLine(normalOrigin, normalDirection);
		normalLine.AddDrawInfo(vertexList, YELLOW);
	}

	//accessors
	float GetOffset() const { return offset; }
	Vector2 GetNormal() const {	return body->position; }
	ObjectType GetType() const { return HALFSPACE ; }
};

// Circle //
class Circle : public Shape
{
private:
	//calculates the circle's moment of inertia from its internal values
	float CalculateInverseMomentOfInertia() const
	{
		//moment of inertia for a disk: I = 0.5mr^2
		return 1.0f / (0.5f * body->GetMass() * radius * radius);
	}

	//Circle is stored as origin at centre, and radius
protected:
	float radius;

public:
	//draw a circle with centre at 10,10, radius of 5
	Circle(): Shape(Vector2(10,10)), radius(5.0f) { body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//give position of centre, circle is there
	Circle(const Vector2 newPos): 
		radius(5.0f), Shape(newPos){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//give position and radius, get circle with properties
	Circle(const Vector2 newPos, const float newRad):
		radius(newRad), Shape(newPos){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//accessor methods
	float GetRadius() const { return radius; }
	ObjectType GetType() const { return CIRCLE;}

	//draw the circle
	void AddDrawInfo(VertexList &vertexList) const 
	{
		//draw a nice smooth circle regardless of radius - this is just the number of points used to draw 
		const int smoothness = (int)radius+25;

		//create the vector of vertices
		std::vector<Vertex> vertices;
		Vertex vertex = {MetresToPixels(body->position.x),MetresToPixels(body->position.y), 0, 1, WHITE};
			
		//values for calculating next point on circumference
		float wedgeAngle = 2*Pi / smoothness;
		float theta;

		//rotate to next point and put into vertices vector
		for (int i =0; i<=smoothness; i++)
		{
			theta = wedgeAngle*i;

			vertex.x = MetresToPixels(body->position.x + radius * cos(theta));
			vertex.y = MetresToPixels(body->position.y - radius * sin(theta));

			vertices.push_back(vertex);
		}

		vertexList.push_back(vertices);
	}

	void RotateAboutWorldOrigin(const float rotation )
	{
		body->position.RotateAboutWorldOrigin(rotation);
	}

	bool IsTheSameAs(const Circle otherCircle)
	{
		if (this->body == otherCircle.body && this->radius == otherCircle.radius)
			return true;

		return false;
	}

	void SetBody(const Shape shape)
	{
		this->body = shape.GetBody();
	}
};

// Old UserShape classes had bounds in place so they could not leave the screen.
// Current version just manipulates regular shapes. 
// UserCircle
/*
//Circle class designed for control by user - ie has bounds. Can use circle for user control but not advised
class UserCircle: public Circle
{
private:
	const int xBound;	//bounds keep user controlled circle inside screen
	const int yBound;

public:
	//no parameters, get default circle and default bounds
	UserCircle():Circle(), xBound(60), yBound(40){}; //TODO: magic numbers :(

	//provide a radius and bounds, they get set
	UserCircle(const float newRad, const int newXBound, const int newYBound): 
	Circle(Vector2(20.0f,20.0f), newRad), xBound(newXBound), yBound(newYBound) {}

	//provide new bounds, get default circle and these bounds
	UserCircle(const int newXBound, const int newYBound): Circle(Vector2(20.0f,20.0f), 2.0f), xBound(newXBound), yBound(newYBound){};


	void Translate(const Vector2 translation)
	{
		body->position += translation;
		KeepWithinBounds();
	}

	void Translate(const float x, const float y)
	{
		body->position+=Vector2(x,y);
		KeepWithinBounds();
	}

	void KeepWithinBounds() 
	{
		if (body->position.x >= xBound-radius)
			body->position.x= (float)xBound-radius;

		if (body->position.x <= 0+radius)
			body->position.x =0+radius;

		if (body->position.y >= yBound-radius)
			body->position.y = (float)yBound-radius;

		if (body->position.y <= 0+radius)
			body->position.y = 0+radius;
	}

};*/


// Box //

class Box : public Shape
{
private:
	//calculates the box's moment of inertia from its internal values
	float CalculateInverseMomentOfInertia() const
	{
		//moment of inertia for a rectangle: I = 1/12m(dx^2 + dy^2)
		return 1/( (1.0f/12.0f) * body->GetMass() * ( GetSize().x*GetSize().x + GetSize().y*GetSize().y ));
	}

protected:
	Vector2 halfSize; //half-size of the box

public:
	//no parameters creates a square 8m big at (6,6)
	Box(): Shape(Vector2(6,6)), halfSize(4,4) {body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//providing a position creates a square 8m big at the position
	Box(const Vector2 newPos): 
		Shape(newPos), halfSize(4,4){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//provide a position and size to create square at place and size
	Box(const Vector2 newPos, const float newSize): 
		Shape(newPos),halfSize(newSize/2.0f,newSize/2.0f){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//provide position height and width for custom rectangle at position
	Box(const Vector2 newPos, const float newHeight, const float newWidth): 
		Shape(newPos), halfSize(newHeight/2.0f, newWidth/2.0f){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//provide position, height, width and rotation to have a rotated square of any size wherever
	Box(const Vector2 newPos, const float newHeight, const float newWidth, const float newOrientation): 
		Shape(newPos, newOrientation), halfSize(newHeight/2.0f, newWidth/2.0f){ body->inverseMomentOfInertia = CalculateInverseMomentOfInertia(); };

	//Draws a box, calculating rotated vertices from rotation member
	void AddDrawInfo(VertexList &vertexList) const
	{
		//get array of vertices
		Vector2 vertsArray[4] ;
		GetVertices(vertsArray);

		std::vector<Vertex> vertices;

		//populate first vertex
		Vertex vertex = {MetresToPixels(vertsArray[0].x), MetresToPixels(vertsArray[0].y), 0, 1, WHITE};
		vertices.push_back(vertex);

		//get other vertices
		for (int i = 1; i<4; i++)
		{
			vertex.x = MetresToPixels(vertsArray[i].x);
			vertex.y = MetresToPixels(vertsArray[i].y);
			vertices.push_back(vertex);
		}

		//put first vertex in again to complete box drawing
		vertex.x = MetresToPixels(vertsArray[0].x);
		vertex.y = MetresToPixels(vertsArray[0].y);
		vertices.push_back(vertex);

		//push onto vertex buffer
		vertexList.push_back(vertices);
	}

	//accessors
	Vector2 GetSize() const { return halfSize*2; }
	Vector2 GetHalfSize() const { return halfSize; }
	ObjectType GetType() const { return BOX; }

	//supply four Vector2s and have the vertex co ordinates put into them for the box's position and rotation
	void GetVertices(Vector2 vertices[]) const 
	{
		//ensure array is correct size 
		if (sizeof(vertices) != 4)	{ throw std::exception("GetVertices(): Array is incorrect size."); }

		//get vertices for unrotated box
		vertices[0] = Vector2(body->position.x-halfSize.x, body->position.y-halfSize.y);	//top left
		vertices[1] = Vector2(body->position.x+halfSize.x, body->position.y-halfSize.y);	//top right
		vertices[2] = Vector2(body->position.x+halfSize.x, body->position.y+halfSize.y);	//bottom right
		vertices[3] = Vector2(body->position.x-halfSize.x, body->position.y+halfSize.y);	//bottom left

		//early outs

		//if no rotation then current values are fine
		if (body->orientation == 0)
		{ 
			return; 
		}
		else if (body->orientation == 180)
		{
			//if rotetion is 180, just swap the opposite corners
			Vector2 temp = vertices[0];

			vertices[0] = vertices[2];
			vertices[2] = temp;

			temp = vertices[1];

			vertices[1] = vertices[3];
			vertices[3] = temp;

			return;
		}		
		else if(body->orientation == 90 && halfSize.x == halfSize.y)
		{
			//if rotation is 90 and box is square, just move them all around clockwise
			Vector2 temp = vertices[0];
			vertices[0] = vertices[3];
			vertices[3] = vertices[2];
			vertices[2] = vertices[1];
			vertices[1] = temp;

			return;
		}
		else if (body->orientation == 270 && halfSize.x == halfSize.y)
		{
			//if rotation is 270 and box is square, move them all around anti clockwise
			Vector2 temp = vertices[0];
			vertices[0] = vertices[1];
			vertices[1] = vertices[2];
			vertices[2] = vertices[3];
			vertices[3] = temp;

			return;
		}

		//rotate each vertex
		for (int i = 0; i < 4; i++)
		{
			vertices[i].RotateAboutPoint(body->position, body->orientation);
		}

	}

	//gets box's local x axis
	Vector2 GetXAxis() const
	{
		Vector2 axis(1,0);
		axis.RotateAboutWorldOrigin(body->orientation);
		return axis;
	}

	//gets box's local y axis
	Vector2 GetYAxis() const
	{
		Vector2 axis(0,1);
		axis.RotateAboutWorldOrigin(body->orientation);
		return axis;
	}

	//rotate box around the world origin
	void RotateAboutWorldOrigin(const float newRot)
	{
		body->position.RotateAboutWorldOrigin(newRot);
		Rotate(newRot);
	}

	bool IsTheSameAs(const Box otherBox)
	{
		if (this->body == otherBox.body && this->halfSize.x == otherBox.halfSize.x && this->halfSize.y == otherBox.halfSize.y)
			return true;
		
		return false;
	}

	void SetBody(const Shape shape)
	{
		this->body = shape.GetBody();
	}
};

// UserBox
////Box class designd for control by user
//class UserBox : public Box
//{
//private:
//	const int xBound; //bounds keep user controlled box within screen
//	const int yBound;
//
//public:
//	//get default box and default bounds
//	UserBox(): Box(), xBound(60), yBound(40){};
//
//	//get default square of size, with new bounds
//	UserBox(const float newSize, const int newXBound, const int newYBound): 
//	Box(Vector2(20.0f, 20.0f), newSize), xBound(newXBound), yBound(newYBound){};
//
//	UserBox(const float newWidth, const float newHeight, const int newXBound, const int newYBound):
//	Box(Vector2(20.0f, 20.0f), newHeight, newWidth), xBound(newXBound), yBound(newYBound){};
//
//	void Translate(const Vector2 translation)
//	{
//		body->position += translation;
//		KeepWithinBounds();
//	}
//
//	void Translate(const float x, const float y)
//	{
//		body->position += Vector2(x,y);
//		KeepWithinBounds();
//	}
//
//	void KeepWithinBounds()
//	{
//		if (body->position.x >= xBound-halfSize.x)
//			body->position.x= (float)xBound-halfSize.x;
//
//		if (body->position.x <= 0)
//			body->position.x =0;
//
//		if (body->position.y >= yBound-halfSize.y)
//			body->position.y = (float)yBound-halfSize.y;
//
//		if (body->position.y <= 0)
//			body->position.y = 0;
//	}
//};


//SHAPEH
#endif 