// Jacob Mills Physics Engine //

// General structure of engine and algorithms adapted to 2D from Game Physics Engine Development, Ian Millington
// Tutorial adapted for creating window and linking Direct3D device: http://www.riemers.net/eng/Tutorials/DirectX/C++/Series1/tut1.php

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

#ifndef TIMEH
#define TIMEH
	#include <ctime>
#endif

#include <windows.h>

#include <dinput.h>

/*#include "core.h"*/
#include "collision.h"
#include "vertex.h"
#include "main.h"

//TODO: replace vectors with Lists if don't have to use [i] for efficiency

// Main Program //

//starting point of application
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR cmdLine, int cmdShow)
{
	// Window and Device Initialisation //

	//create window
	HWND window = NewWindow("Jacob Mills Physics Engine", 50, 50, screenWidth, screenHeight);

	//pointer to the Direct3D device
	Device device = InitialiseDevice(window);

	//messages from OS
	MSG message;

	//get pointer to keyboard device for input
	InputDevice keyboard = InitialiseKeyboard(window);

	//set up font for text
	LPD3DXFONT font = InitialiseFont(device);

	//start the clocks
	std::clock_t velocityResolutionStart;
	double velocityResolutionDuration;

	std::clock_t userShapeToggleStart;
	double userShapeToggleDuration;

	std::clock_t penetrationResolutionStart;
	double penetrationResolutionDuration;


	velocityResolutionStart = std::clock();
	userShapeToggleStart = std::clock();
	penetrationResolutionStart = std::clock();

	
	// Engine Initialisation //
	
	//text to be displayed on the screen
	std::string screenText;

	//vector holds vertex information for every shape in world, for drawing
	VertexList vertexList;	
	
	//contact generation
	CollisionDetector collisionDetector;
	std::vector<Contact> collisionList;
	ObjectList collidableObjects;


	//get gridlines
	std::vector<DrawLine> gridlines;
	InitialiseGridlines(gridlines);

	//set up halfspace border
	std::vector<HalfSpace> border;
	InitialiseHalfSpaceBorder(border);
	for (unsigned i = 0; i<border.size(); i++)
	{
		collidableObjects.Add(border[i]);
	}


	// Shape Initialisation //
	Circle circle(Vector2(32,12), 4.0f);
	collidableObjects.Add(circle);

	Box box(Vector2(20,12), 5, 8);	
	collidableObjects.Add(box);

	Box box2(Vector2(30, 22), 7, 9);
	collidableObjects.Add(box2);
	box2.SetVelocity(-10, 0);
	box2.SetMass(20);

	HalfSpace halfSpace(Vector2(1,1), 15);
	collidableObjects.Add(halfSpace);

	// Screen Text //
	std::string previousContactInfo = "No contacts made";
	std::string previousPenetrationResolutionText = "No penetrations resolved";
	std::string previousVelocityResolutionText = "No velocities resolved";


	// UserShapes //
	//both objects are created, but the userShapeIsBox bool controls whether either is collidable or being drawn - at start, box is collidable and drawn, circle is not.
	Box userBox;
	collidableObjects.Add(userBox);

	Circle userCircle;


	//  Main Loop  //

	// Control Booleans //
	//controls main loop
	bool running = true;

	bool resolvePenetrationsL = false;
	bool resolvePenetrationsNL = false;

	bool resolveVelocities = false;
	bool resolveVelocitiesAndRotations = false;

	unsigned displayedText = 1;
	bool helpText = true;

	bool userShapeToggle = false; 
	bool userShapeIsBox = true;

	bool showVelocitiesAndRotations = true;
	
	while (running)
	{
		// System //
		//ask OS if there are any messages for window; when read, remove it
		if(PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			if(!IsDialogMessage(window, &message))
			{
				//pass message on to window
				DispatchMessage(&message);
			}
		}

		// Draw Background //
		// Gridlines
		DrawGrid(gridlines, vertexList);

		// Border
		DrawHalfSpaceBorderWithNormals(border, vertexList);

		if (helpText)
		{
			screenText += " Controls: \n";
			screenText += "  Arrow keys - move userShape . . . <,> - rotate userShape . . . tab - toggle userShape between circle and box\n";
			screenText += "  l - linear position resolution . . . p - try non-linear projection\n";
			screenText += "  v - resolve linear velocity . . . g - try to resolve linear and angular velocity\n";
			screenText += "  1 - display contact information . . . 2 - display penetration resolution information . . . 3 - display velocity resolution information\n";
			screenText += "  q - hide velocities and rotations . . . t - hide help text\n\n";
		}

		//  Update  //
		// Input 
		Vector2 userTranslation;
		float userRotation;
		ReadKeyboard(keyboard, userTranslation, userRotation, running, resolvePenetrationsL, resolvePenetrationsNL, resolveVelocities, resolveVelocitiesAndRotations, displayedText, userShapeToggle, helpText, showVelocitiesAndRotations);

		//switch user shape from box to circle or vice versa
		if (userShapeToggle)
		{
			userShapeToggleDuration = (std::clock() - userShapeToggleStart) / (double)CLOCKS_PER_SEC;
			if (userShapeToggleDuration > 1)
			{
				if (userShapeIsBox)
				{
					//switch to circle
					collidableObjects.Remove(userBox);
					collidableObjects.Add(userCircle);

					userCircle.SetBody(userBox.GetBody());
					userShapeIsBox = false;

				}
				else
				{
					//switch to box
					collidableObjects.Remove(userCircle);
					collidableObjects.Add(userBox);

					userBox.SetBody(userCircle.GetBody());

					userShapeIsBox = true;
				}
				
				//reset the clock
				userShapeToggleStart = std::clock();
			}

			userShapeToggle = false; 
		}

		//move the user shape as input
		if (userShapeIsBox)
		{
			userBox.Translate(userTranslation);
			userBox.Rotate(userRotation);
		}
		else
		{
			userCircle.Translate(userTranslation);
			userCircle.Rotate(userRotation);
		}

		// Collision Detection //
		unsigned numOfCollisions = collisionDetector.GenerateContacts(collidableObjects, collisionList);

		// Text display
		// mode 1 is display information about the userShape's current or most recent collision
		if (displayedText == 1)
		{	
			screenText += "Number of collisions: ";
			screenText += ToString((float)numOfCollisions);
			screenText += "\n";


			std::string contactInfo ;

			//find the usershape in the list of collisions, and get its contact info text
			if (userShapeIsBox)
			{
				for (unsigned i = 0; i < collisionList.size(); i++)
				{
					if (userBox.BodySameAs(collisionList[i].GetBody(0)) || userBox.BodySameAs(collisionList[i].GetBody(1)))
					{
						contactInfo += collisionList[i].GetContactInfoText();
					}
				}
			}
			else
			{
				for (unsigned i = 0; i < collisionList.size(); i++)
				{
					if (userCircle.BodySameAs(collisionList[i].GetBody(0)) || userCircle.BodySameAs(collisionList[i].GetBody(1)))
					{
						contactInfo += collisionList[i].GetContactInfoText();
					}
				}
			}

			//if contactInfo isn't empty, some data was recorded, so use it
			if (contactInfo != "")
			{
				screenText += "\nCurrent collision data: \n\n";
				screenText += contactInfo;
				previousContactInfo = contactInfo;
			}
			else //else there was nothing, just display the most recent collision data
			{
				screenText += "\nPrevious collision data: \n\n";
				screenText += previousContactInfo;
			}

		}


		std::string penetrationResolutionText;
		std::string velocityResolutionText;

		//resolve penetrations
		if (numOfCollisions > 0)
		{
			if (resolvePenetrationsNL)
			{
				//protect it from doing it too often
				penetrationResolutionDuration =  (std::clock() - penetrationResolutionStart) / (double)CLOCKS_PER_SEC;
				if (penetrationResolutionDuration > 1)
				{
					// Text display
					// mode 2 is show before and after values for most recent resolved penetrations
					if (displayedText == 2)
					{
						penetrationResolutionText += "Penetration resolution information: \n";
						penetrationResolutionText += "\nUserShape pre-resolution: \n";

						if (userShapeIsBox)
							penetrationResolutionText += userBox.GetPositionInfoText();
						else
							penetrationResolutionText += userCircle.GetPositionInfoText();
					}

					for (unsigned i = 0; i<numOfCollisions; i++)
					{
						collisionList[i].ResolvePositionWithRotation();
					}
					resolvePenetrationsNL = false;

					// Text display
					if (displayedText == 2)
					{
						penetrationResolutionText += "\nUserShape post-resolution: \n";

						if (userShapeIsBox)
							penetrationResolutionText += userBox.GetPositionInfoText();
						else
							penetrationResolutionText += userCircle.GetPositionInfoText();
					}

					penetrationResolutionStart = std::clock();
				}
			}
			else if (resolvePenetrationsL)
			{
				penetrationResolutionDuration =  (std::clock() - penetrationResolutionStart) / (double)CLOCKS_PER_SEC;
				if (penetrationResolutionDuration > 1)
				{
					// Text display
					// mode 2 is show before and after values for most recent resolved penetrations
					if (displayedText == 2)
					{
						penetrationResolutionText += "Penetration resolution information: \n";
						penetrationResolutionText += "\nUserShape pre-resolution: \n";

						if (userShapeIsBox)
							penetrationResolutionText += userBox.GetPositionInfoText();
						else
							penetrationResolutionText += userCircle.GetPositionInfoText();
					}

					for (unsigned i = 0; i<numOfCollisions; i++)
					{
						collisionList[i].ResolvePosition();
					}
					resolvePenetrationsL = false;

					// Text display
					if (displayedText == 2)
					{
						penetrationResolutionText += "\nUserShape post-resolution: \n";

						if (userShapeIsBox)
							penetrationResolutionText += userBox.GetPositionInfoText();
						else
							penetrationResolutionText += userCircle.GetPositionInfoText();
					}
				}
				penetrationResolutionStart = std::clock();
			}

		

			if (resolveVelocities)
			{
				//protect it from doing it too often
				velocityResolutionDuration = (std::clock() - velocityResolutionStart) / (double)CLOCKS_PER_SEC;
				if(velocityResolutionDuration > 1)
				{
					// Text display
					// mode 3 is show before and after values for most recent resolved velocities
					if (displayedText == 3)
					{
						velocityResolutionText += "Velocity resolution information: \n";
						velocityResolutionText += "\nUserShape pre-resolution: \n";

						if (userShapeIsBox)
							velocityResolutionText += userBox.GetVelocityInfoText();
						else
							velocityResolutionText += userCircle.GetVelocityInfoText();
					}

					for (unsigned i = 0; i<numOfCollisions; i++)
					{
						collisionList[i].ResolveVelocities();
						velocityResolutionStart = std::clock();
					}

					// Text display
					if (displayedText == 3)
					{
						velocityResolutionText += "\nUserShape post-resolution: \n";

						if (userShapeIsBox)
							velocityResolutionText += userBox.GetVelocityInfoText();
						else
							velocityResolutionText += userCircle.GetVelocityInfoText();
					}

				}

				resolveVelocities = false;
			}
			else if (resolveVelocitiesAndRotations)
			{
				//protect it from doing it too often
				velocityResolutionDuration = (std::clock() - velocityResolutionStart) / (double)CLOCKS_PER_SEC;
				if(velocityResolutionDuration > 1)
				{
					// Text display
					if (displayedText == 3)
					{
						velocityResolutionText += "Velocity resolution information: \n";
						velocityResolutionText += "\nUserShape pre-resolution: \n";

						if (userShapeIsBox)
							velocityResolutionText += userBox.GetVelocityInfoText();
						else
							velocityResolutionText += userCircle.GetVelocityInfoText();
					}

					for (unsigned i = 0; i<numOfCollisions; i++)
					{
						collisionList[i].ResolveVelocitiesAndRotations();
						velocityResolutionStart = std::clock();
					}

					// Text display
					if (displayedText == 3)
					{
						velocityResolutionText += "\nUserShape post-resolution: \n";

						if (userShapeIsBox)
							velocityResolutionText += userBox.GetVelocityInfoText();
						else
							velocityResolutionText += userCircle.GetVelocityInfoText();
					}

				}
					resolveVelocitiesAndRotations = false;
			}
		}

		// Text display
		// if text mode is 2 or 3, check there is some new text, and if so, display that
		if (displayedText == 2 && penetrationResolutionText != "")
		{
			screenText += penetrationResolutionText;
			previousPenetrationResolutionText = penetrationResolutionText;
		}
		else if (displayedText == 2)
		{
			screenText += previousPenetrationResolutionText;
		}

		if (displayedText == 3 && velocityResolutionText!="")
		{
			screenText += velocityResolutionText;
			previousVelocityResolutionText = velocityResolutionText;
		}
		else if (displayedText ==3)
		{
			screenText += previousVelocityResolutionText;
		}


		//  Draw  //

		// Draw Shapes
		halfSpace.AddDrawInfo(vertexList);
		halfSpace.DrawNormal(vertexList);

		box.AddDrawInfo(vertexList);
		box2.AddDrawInfo(vertexList);
		
		circle.AddDrawInfo(vertexList);

		if (userShapeIsBox)
			userBox.AddDrawInfo(vertexList);
		else
			userCircle.AddDrawInfo(vertexList);
		
		// if toggled on, show lines representing velocity and rotation
		if (showVelocitiesAndRotations)
		{
			box.DrawVelocityAndRotation(vertexList);
			box2.DrawVelocityAndRotation(vertexList);
			circle.DrawVelocityAndRotation(vertexList);

			if (userShapeIsBox)
				userBox.DrawVelocityAndRotation(vertexList);
			else
				userCircle.DrawVelocityAndRotation(vertexList);

		}
		

		// Draw Contacts
		collisionDetector.DrawContacts(collisionList, vertexList);

		// Draw All //
		//Fills Direct3D vertex buffer, and draws vertices
		Draw(window, device, vertexList, font, screenText);

		//  Housekeeping  //
		//clear data in vertexBuffer, screenText and collisions so that they don't get added to every loop
		vertexList.clear();
		screenText.clear();
		collisionList.clear();
	}

	// End application //

	//release the keyboard
	keyboard->Unacquire();
	keyboard->Release();

	font->Release();

	//release memory from Direct3D device
	device->Release();
	
	//destroy the window
	DestroyWindow(window);

	//end of program
	return 0;
}

//set up keyboard device
InputDevice InitialiseKeyboard(HWND window)
{
	//kb = keyboard
	LPDIRECTINPUT8 kbObject;
	InputDevice kbDevice;

	//create the DirectInput object
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
		IID_IDirectInput8, (void**)&kbObject, NULL);

	//create the keyboard device
	kbObject->CreateDevice(GUID_SysKeyboard, &kbDevice, NULL);

	//use default data format
	kbDevice->SetDataFormat(&c_dfDIKeyboard);

	//bind to window, only listen when active
	kbDevice->SetCooperativeLevel(window, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	
	//gets access to keyboard
	kbDevice->Acquire();

	//return pointer to keyboard device
	return kbDevice;
}

//get state of keyboard
void ReadKeyboard(InputDevice keyboard, Vector2 &translation, float &rotation, bool &running, bool &resolvePenetrationsL, bool &resolvePenetrationsNL, bool &resolveVelocities, bool &resolveVelocitiesAndRotations, unsigned &displayedText, bool &userShape, bool &helpText, bool &showVelocitiesAndRotations)
{
	translation = Vector2(0,0);
	rotation = 0;

	//buffer to hold keyboard data
	char keyboardState[256];
	
	if(FAILED(
		keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState) //put the keyboard state into keyboardState
		))
	{
		keyboard->Acquire(); //if failed to get keyboardState, acquire the keyboard
	}


	//the way this works is - each key stored as a byte. first bit of that data
	//is 0 if key up, 1 if key down. ERGO, divide the key by 128 to get input state
	//program running state
	if (keyboardState[DIK_ESCAPE]/128)
		running = 0;

	//shape translation
	if (keyboardState[DIK_LEFT]/128)
		translation.x -= 0.5f;

	if (keyboardState[DIK_RIGHT]/128)
		translation.x += 0.5f;
		
	if (keyboardState[DIK_UP]/128)
		translation.y -= 0.5f;

	if (keyboardState[DIK_DOWN]/128)
		translation.y += 0.5f;

	//shape rotation
	if (keyboardState[DIK_COMMA]/128)
		rotation -= 2.0f;

	if (keyboardState[DIK_PERIOD]/128)
		rotation += 2.0f;

	//position resolution toggle
	if (keyboardState[DIK_P]/128)
		resolvePenetrationsNL = true;
	else
		resolvePenetrationsNL = false;

	if (keyboardState[DIK_L]/128)
		resolvePenetrationsL = true;
	else
		resolvePenetrationsL = false;

	//velocity resolution toggle
	if (keyboardState[DIK_V]/128)
		resolveVelocities = true;
	else
		resolveVelocities = false;

	if (keyboardState[DIK_G]/128)
		resolveVelocitiesAndRotations = true;
	else
		resolveVelocitiesAndRotations = false;

	//screenText info toggle
	if (keyboardState[DIK_1]/128)
		displayedText = 1;
	else if (keyboardState[DIK_2]/128)
		displayedText = 2;
	else if (keyboardState[DIK_3]/128)
		displayedText = 3;
	

	//user shape toggle: true = box, false = circle
	if (keyboardState[DIK_TAB]/128)
		userShape = !userShape;

	//toggle the help text on or off
	if (keyboardState[DIK_T]/128)
		helpText = !helpText;

	if (keyboardState[DIK_Q]/128)
		showVelocitiesAndRotations = !showVelocitiesAndRotations;
	
}

//initialises font for use
LPD3DXFONT InitialiseFont(Device device)
{
	LPD3DXFONT font;

	//create the object (size 12 bold arial)
	D3DXCreateFont(device, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, TEXT("Arial"), &font);
	
	return font;
}

//draws the text passed in, with passed font
void DisplayText(LPD3DXFONT font, LPCTSTR text)
{
	//ready the text area
	RECT area;
	area.left = 7;
	area.right = 1100;
	area.top = 7;
	area.bottom = 700;

	//colour for the font is white
	D3DCOLOR colour = D3DCOLOR_ARGB(255,255,255,255);

	font->DrawText(NULL, text, -1, &area, DT_LEFT|DT_WORDBREAK, colour);
}

//create a new window with a title, x and y positions on screen, width and height
HWND NewWindow(LPCTSTR title, int xPos, int yPos, int width, int height)
{
	WNDCLASSEX window;

	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_HREDRAW | CS_VREDRAW;	//redraw window if moved or resized?
	window.lpfnWndProc = WindowProcedure; //called when something window related happens

	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hInstance = GetModuleHandle(NULL);
	window.hIcon = NULL; //handle to application image
	window.hCursor = NULL; //handle to a cursor image
	window.hbrBackground = GetSysColorBrush(COLOR_BTNFACE); //background colour
	window.lpszMenuName = NULL;
	window.lpszClassName = "ClassName";
	window.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

	RegisterClassEx(&window);

	//create window, return handle to it
	return CreateWindowEx(WS_EX_CONTROLPARENT, "ClassName", title, 
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		xPos, yPos, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
}

//called when an event such as keypress occurs - but input is done through DirectInput
//so just return default windows procedure
LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(window, message, wParam, lParam);
}

//links Direct3D device to screen
Device InitialiseDevice(HWND windowToBindTo)
{
	//check capabilities of graphics card, ensure DirectX installed on machine
	LPDIRECT3D9 direct3DObject;

	direct3DObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (direct3DObject == NULL)
	{
		MessageBox(windowToBindTo, "DirectX runtime library isn't installed.", "InitialiseDevice()", MB_OK);
		exit(1);
	}

	//Direct3D parameters structure
	D3DPRESENT_PARAMETERS presentParams;

	ZeroMemory(&presentParams, sizeof(presentParams)); //clear structure
	presentParams.Windowed = TRUE; //no fullscreen
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.BackBufferFormat = D3DFMT_UNKNOWN;

	//link the device, return pointer to it
	Device device;
	if (FAILED(
		direct3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowToBindTo, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, &device)
		))
	{
		//if hardware not good enough, link to the software emulator instead
		if(FAILED(
			direct3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, windowToBindTo, 
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &device)
			))
		{
			//if the software emulator link fails too
			MessageBox(windowToBindTo, "Failed to create the reference device.", "Initialisedevice()", MB_OK);
			exit(1);
		}
	}

	//return the pointer to the device
	return device;
}

//edit this so that a shape can add to vertex buffer each frame with its updated position
void Draw( HWND window, LPDIRECT3DDEVICE9 device, VertexList &vertexList, LPD3DXFONT font, std::string text )
{
	// Populate the vertex buffer //

	//number of shapes is the number of lists of vertices
	const int noOfShapes = vertexList.size();

	std::vector<UINT> starts;
	std::vector<UINT> counts;

	//number of vertices is the total number of all vertices in all shapes
	int noOfVertices = 0;
	UINT prevCount = 0;
	for (int i = 0; i<noOfShapes; i++)
	{
		//find the number of vertices in the shape, then calculate the start and count values for drawing
		UINT vertsInShape = vertexList[i].size();
		starts.push_back(i+prevCount);
		counts.push_back(vertsInShape -1);
		//add the number of vertices in this shape to the count, and also the total number of vertices
		prevCount += vertsInShape -1 ;
		noOfVertices += vertsInShape;
	}

	//create a vector of all the vertices
	std::vector<Vertex> vertices;
	for (int i=0;i<noOfShapes; i++)
	{
		for (unsigned int j =0; j<vertexList[i].size(); j++)
		{
			vertices.push_back(vertexList[i][j]);
		}
	}

	//create vertex buffer for data
	VertexBuffer vertexBuffer;
	if(FAILED(
		device->CreateVertexBuffer(/*size*/noOfVertices*sizeof(Vertex),0,
		D3DFVF_XYZRHW|D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vertexBuffer , NULL)
		))
	{
		//display message if memory reservation fails
		MessageBox(window, "Error creating vertex buffer","FillVertices()", MB_OK);
		exit(1);
	}

	//pointer to memory
	VOID* verticesPtr;
	//lock allocated memory for vertex buffer
	if(FAILED(
		vertexBuffer->Lock(0,noOfVertices*sizeof(Vertex), (void**)&verticesPtr, 0)
		))
	{
		//display message if fails
		MessageBox(window, "Error trying to lock memory","FillVertices()",MB_OK);
		exit(1);
	}
	else
	{
		//if memory allocation successful, copy data to locked memory
		memcpy(verticesPtr, &vertices[0], /*size*/noOfVertices*sizeof(Vertex));
		
		//unlock vertex buffer for other applications
		vertexBuffer->Unlock();
	}
	

	// Draw the scene //

	//clear the whole window
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,200), 1.0f, 0);

	//begin
	device->BeginScene();

	//get data from vertex buffer
	device->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
	device->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);						//rhw means do things in screen space rather than world space

	//for each shape
	for(int i= 0; i<noOfShapes; i++)
	{		
		//draw the data in vertex buffer
		device->DrawPrimitive(D3DPT_LINESTRIP, starts[i], counts[i]);		
	}

	//display the text
	DisplayText(font, text.c_str());
	
	//end
	device->EndScene();

	//present the buffered new screen
	device->Present(NULL,NULL,NULL,NULL);

	//free up vertex buffer memory 
	vertexBuffer->Release();
}

//fills passed in vector with information for gridlines
void InitialiseGridlines(std::vector<DrawLine> &gridlines) 
{
	//get number of horizontal and vertical lines needed from screen info and pixel-metre ratio
	//one line drawn every 1m, so same as screen height and width in metres
	const int horizLines = (int)PixelsToMetres((float)screenHeight);
	const int vertiLines = (int)PixelsToMetres((float)screenWidth);

	//for every line
	for (int i = 0; i<(horizLines+vertiLines); i++)
	{
		if(i < vertiLines) //if its a vertical line, draw it on the x axis
			gridlines.push_back(DrawLine(Vector2((float)i,0), (float)horizLines, 90));
		else //else on the y axis
			gridlines.push_back(DrawLine(Vector2(0,(float)i-vertiLines), (float)vertiLines));
	}
}

//pushes gridlines onto vertex buffer for drawing
void DrawGrid( std::vector<DrawLine> &gridlines, VertexList &vertexList ) 
{
	for (unsigned int i =0; i < gridlines.size(); i++)
	{	
		//draw each gridline, in a nice blue so as not to contrast the background
		gridlines[i].AddDrawInfo(vertexList, 0xff3333ff);
	}
}

//fills a vector with four half spaces, that match the size of the screen
void InitialiseHalfSpaceBorder(std::vector<HalfSpace> &border)
{
	HalfSpace halfSpaceTop(Vector2(0.0f, 1.0f), 1.0f);
	HalfSpace halfSpaceRight(Vector2(-1.0f, 0.0f), -(PixelsToMetres((float)screenWidth) - 1.0f));
	HalfSpace halfSpaceBottom(Vector2(0.0f, -1.0f), -(PixelsToMetres((float)screenHeight) -2.0f));
	HalfSpace halfSpaceLeft(Vector2(1.0f,0.0f), 1.0f);

	border.push_back(halfSpaceTop);
	border.push_back(halfSpaceRight);
	border.push_back(halfSpaceBottom);
	border.push_back(halfSpaceLeft);

}

//draw the half spaces that make up the border
void DrawHalfSpaceBorder( std::vector<HalfSpace> &border, VertexList &vertexList ) 
{
	for(unsigned int i =0; i<border.size(); i++)
	{
		border[i].AddDrawInfo(vertexList);
	}
}

//draw the half spaces for the border, and their normals as well
void DrawHalfSpaceBorderWithNormals( std::vector<HalfSpace> &border, VertexList &vertexList ) 
{
	for(unsigned int i =0; i<border.size(); i++)
	{
		border[i].AddDrawInfo(vertexList);
		border[i].DrawNormal(vertexList);
	}
}