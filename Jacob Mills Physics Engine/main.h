// Constants //
// Specifying window height and width
const int screenHeight = 800;
const int screenWidth = 1200;

// Declarations //

// Windows
HWND NewWindow(LPCTSTR title, int xPos, int yPos, int width, int height);
LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM parameter1, LPARAM parameter2);
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR cmdLine, int cmdShow);

// Direct3D
LPDIRECT3DDEVICE9 InitialiseDevice(HWND windowToBindTo);

//Input
InputDevice InitialiseKeyboard(HWND window);
void ReadKeyboard(InputDevice keyboard, Vector2 &translation, float &rotation, bool &running, bool &resolvePenetrationsL, bool &resolvePenetrationsNL, bool &resolveVelocities, bool &resolveVelocitiesAndRotations, unsigned &displayedText, bool &userShape, bool &helpText, bool &showVelocitiesAndRotations );

// Drawing
void Draw( HWND window, LPDIRECT3DDEVICE9 device, VertexList &vertexList, LPD3DXFONT font, std::string text );
void DrawGrid( std::vector<DrawLine> &gridlines, VertexList &vertexList );
void InitialiseGridlines(std::vector<DrawLine> &gridlines);

// HalfSpace Border
void InitialiseHalfSpaceBorder(std::vector<HalfSpace> &border);
void DrawHalfSpaceBorder( std::vector<HalfSpace> &border, VertexList &vertexList );
void DrawHalfSpaceBorderWithNormals( std::vector<HalfSpace> &border, VertexList &vertexList );

// Text
LPD3DXFONT InitialiseFont(Device device);
void DisplayText(LPD3DXFONT font, LPCTSTR text);