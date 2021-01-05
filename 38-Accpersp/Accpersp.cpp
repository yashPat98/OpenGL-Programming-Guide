// ------------------------
// Name :        Yash Patel
// Assignment :  Accumulation perspective assignment  
// Date :        05-01-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Accpersp.h"

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// --- Macros ---
#define WIN_WIDTH  800                                   //window width
#define WIN_HEIGHT 600                                   //window height

#define ACSIZE 8

// --- Global Function Declaration ---
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);    //callback function

// --- Global Variables ---
DWORD dwStyle;                                           //window style
FILE *gpFile            = NULL;                          //log file
HWND ghwnd              = NULL;                          //global hwnd
HDC ghdc                = NULL;                          //current device context
HGLRC ghrc              = NULL;                          //rendering context
WINDOWPLACEMENT wpPrev  = { sizeof(WINDOWPLACEMENT) };   //window placement before fullscreen

bool gbFullscreen       = false;                         //toggling fullscreen
bool gbActiveWindow     = false;                         //render only if window is active

GLfloat lightPosition[]      = {0.0f, 0.0f, 10.0f, 1.0f};
GLfloat lightModelAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};

GLfloat materialAmbient[]    = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[]   = {1.0f, 1.0f, 1.0f, 1.0f};

GLUquadric *quadric;

typedef struct 
{
    double x, y;
} jitter_point;

jitter_point j8[] =
{
    {-0.334818,  0.435331},
    { 0.286438, -0.393495},
    { 0.459462,  0.141540},
    {-0.414498, -0.192829},
    {-0.183790,  0.082102},
    {-0.079263, -0.317383},
    { 0.102254,  0.299133},
    { 0.164216, -0.054399}
};

// --- WinMain() - entry point function ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function prototypes
    void Initialize(void);
    void Display(void);

    //variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");
    int cxScreen, cyScreen;
    bool bDone = false;

    //code
    //create/open 'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Cannot open 'log.txt' file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "'log.txt' file created successfully.\nProgram started successfully.\n\n");
    }

    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;
    wndclass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    //register above class
    RegisterClassEx(&wndclass);

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    //create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppName,
        TEXT("OpenGL : Accpersp"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (cxScreen - WIN_WIDTH) / 2,
        (cyScreen - WIN_HEIGHT) / 2,
        WIN_WIDTH,
        WIN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ghwnd = hwnd;
    
    Initialize();

    //show window
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //game loop
    while(bDone == false)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                    bDone = true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(gbActiveWindow == true)
            {
                Display();
            }
        }
        
    }

    return ((int)msg.wParam);
}

// --- WndProc() - callback function ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //function prototypes
    void ToggleFullscreen(void);
    void Resize(int, int);
    void UnInitialize(void);

    //code
    switch(iMsg)
    {              
        case WM_SETFOCUS:
            gbActiveWindow = true;
            break;
        
        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:
            return (0);

        case WM_SIZE:
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case 0x46:
                    //fall through
                case 0x66:
                    ToggleFullscreen();
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnInitialize();
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

// --- ToggleFullscreen() - toggle fullscreen ---
void ToggleFullscreen(void)
{
    //variable declaration
    MONITORINFO mi = { sizeof(MONITORINFO) };

    //code
    if(gbFullscreen == false)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }

        ShowCursor(false);
        gbFullscreen = true;
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
        ShowCursor(true);
        gbFullscreen = false;
    }
    
}

// --- Initialize() - initializes rendering context ---
void Initialize(void)
{
    //function prototypes
    void Resize(int, int);

    //variable declaration
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    //code
    ghdc = GetDC(ghwnd);

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    //initialization of PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion    = 1;
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType  = PFD_TYPE_RGBA;
    pfd.cColorBits  = 32;
    pfd.cRedBits    = 8;
    pfd.cBlueBits   = 8;
    pfd.cGreenBits  = 8;
    pfd.cAlphaBits  = 8;
    pfd.cDepthBits  = 32;
    pfd.cAccumBits  = 32;

    //choose required pixel format from device context
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set that pixel format as current
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed.\n");
        DestroyWindow(ghwnd);
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed.\n");
        DestroyWindow(ghwnd);
    }

    // --- Setup Render Scene ---

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //clear accumulation buffer
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);

    //set up light 0 
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);

    //material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

    //enable lighting and material properties
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

// --- Resize() --- 
void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

// --- Display() - renders scene ---
void Display(void)
{
    //function declaration
    void displayObjects(void);
    void accPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus);

    //variable declaration
    GLint viewport[4];
    int jitter;

    //code
    glGetIntegerv(GL_VIEWPORT, viewport);

    glClear(GL_ACCUM_BUFFER_BIT);
    for(jitter = 0; jitter < ACSIZE; jitter++)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        accPerspective(50.0f, (GLdouble)viewport[2] / (GLdouble)viewport[3],
                       1.0f, 15.0f, j8[jitter].x, j8[jitter].y, 
                       0.0f, 0.0f, 1.0f);
        displayObjects();
        glAccum(GL_ACCUM, 1.0f / ACSIZE);
    }
    glAccum(GL_RETURN, 1.0f);

    SwapBuffers(ghdc);
}

void displayObjects(void)
{
    //function declaration
    void DrawTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
    void DrawCube(void);

    //variable declaration
    GLfloat torus_diffuse[] = {0.7f, 0.7f, 0.0f, 1.0f};
    GLfloat cube_diffuse[] = {0.0f, 0.7f, 0.7f, 1.0f};
    GLfloat sphere_diffuse[] = {0.7f, 0.0f, 0.7f, 1.0f};

    //code
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(30.0f, 1.0f, 0.0f, 0.0f);

        //torus
        glPushMatrix();
            glTranslatef(-0.80f, 0.45f, 0.0f);
            glRotatef(100.0f, 1.0f, 0.0f, 0.0f);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, torus_diffuse);
            DrawTorus(1.0f, 0.4f, 30, 30);
        glPopMatrix();

        //cube
        glPushMatrix();
            glTranslatef(-0.75f, -0.50f, 0.0f);
            glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_diffuse);
            DrawCube();
        glPopMatrix();

        //sphere
        glPushMatrix();
            glTranslatef(0.75f, 0.60f, 0.0f);
            glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_diffuse);
            quadric = gluNewQuadric();
            gluSphere(quadric, 1.0f, 30, 30);
        glPopMatrix();
    glPopMatrix();
}

void accPerspective(GLdouble fovy, GLdouble aspect, 
                    GLdouble zNear, GLdouble zFar, 
                    GLdouble pixdx, GLdouble pixdy,
                    GLdouble eyedx, GLdouble eyedy,
                    GLdouble focus)
{
    //function declaration
    void accFrustum(GLdouble left, GLdouble right, 
                    GLdouble bottom, GLdouble top, 
                    GLdouble zNear, GLdouble zFar, 
                    GLdouble pixdx, GLdouble pixdy, 
                    GLdouble eyedx, GLdouble eyedy,
                    GLdouble focus);

    //variable declaration
    GLdouble fov2, left, right, bottom, top;

    //code
    fov2 = ((fovy * M_PI) / 180.0f) / 2.0f;
    
    top = zNear / (cos(fov2) / sin(fov2));
    bottom = -top;
    right = top * aspect;
    left = -right;

    accFrustum(left, right, bottom, top, zNear, zFar, pixdx, pixdy, eyedx, eyedy, focus);
};

void accFrustum(GLdouble left, GLdouble right, 
                GLdouble bottom, GLdouble top, 
                GLdouble zNear, GLdouble zFar, 
                GLdouble pixdx, GLdouble pixdy, 
                GLdouble eyedx, GLdouble eyedy,
                GLdouble focus)
{
    //variable declaration
    GLdouble xwsize, ywsize;
    GLdouble dx, dy;
    GLint viewport[4];

    //code
    glGetIntegerv(GL_VIEWPORT, viewport);

    xwsize = right - left;
    ywsize = top - bottom;
    dx = - (pixdx * xwsize / (GLdouble)viewport[2] + eyedx * zNear / focus);
    dy = - (pixdy * ywsize / (GLdouble)viewport[3] + eyedy * zNear / focus);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(left + dx, right + dx, bottom + dy, top + dy, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(-eyedx, -eyedy, 0.0f);
}

void DrawCube(void)
{
        glBegin(GL_QUADS);
        //near
        glNormal3f(0.0f, 0.0f, 1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        //right
        glNormal3f(1.0f, 0.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        //far
        glNormal3f(-1.0f, 0.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        //left
        glNormal3f(-1.0f, 0.0f, 0.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        //top
        glNormal3f(0.0f, 1.0f, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        //bottom
        glNormal3f(0.0f, -1.0f, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);        
    glEnd();
}

void DrawTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor)
{
    GLfloat vNormal[3];
    double majorStep = 2.0f*M_PI / numMajor;
    double minorStep = 2.0f*M_PI / numMinor;
    int i, j;
	
    for (i=0; i<numMajor; ++i) 
	{
		double a0 = i * majorStep;
		double a1 = a0 + majorStep;
		GLfloat x0 = (GLfloat) cos(a0);
		GLfloat y0 = (GLfloat) sin(a0);
		GLfloat x1 = (GLfloat) cos(a1);
		GLfloat y1 = (GLfloat) sin(a1);
		
		glBegin(GL_TRIANGLE_STRIP);
		for (j=0; j<=numMinor; ++j) 
		{
			double b = j * minorStep;
			GLfloat c = (GLfloat) cos(b);
			GLfloat r = minorRadius * c + majorRadius;
			GLfloat z = minorRadius * (GLfloat) sin(b);
			
			// First point
			glTexCoord2f((float)(i)/(float)(numMajor), (float)(j)/(float)(numMinor));
			vNormal[0] = x0*c;
			vNormal[1] = y0*c;
			vNormal[2] = z/minorRadius;
			
			glNormal3fv(vNormal);
			glVertex3f(x0*r, y0*r, z);
			
			glTexCoord2f((float)(i+1)/(float)(numMajor), (float)(j)/(float)(numMinor));
			vNormal[0] = x1*c;
			vNormal[1] = y1*c;
			vNormal[2] = z/minorRadius;
			
			glNormal3fv(vNormal);
			glVertex3f(x1*r, y1*r, z);
		}
		glEnd();
	}
}


// --- UnInitialize() ---
void UnInitialize(void)
{
    //code
    if(gbFullscreen == true)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
        ShowCursor(true);
    }

    gluDeleteQuadric(quadric);

    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if(gpFile)
    {
        fprintf(gpFile, "\nProgram completed successfully.\n'log.txt' file closed successfully.\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
