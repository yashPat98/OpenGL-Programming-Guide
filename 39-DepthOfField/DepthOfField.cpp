// ------------------------
// Name :        Yash Patel
// Assignment :  Depth Of Field Assignment
// Date :        06-01-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "DepthOfField.h"
#include "Teapot.h"

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// --- Macros ---
#define WIN_WIDTH  800                                   //window width
#define WIN_HEIGHT 600                                   //window height

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

GLfloat lightAmbient[]  = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 3.0f, 3.0f, 0.0f};

GLfloat light_model_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat local_view[] = {0.0f};

GLuint teapotList;
GLfloat current_focus = 4.5f;

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
        TEXT("OpenGL : Depth Of Field"),
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

        case WM_CHAR:
            switch(wParam)
            {
                case '1':
                    current_focus = 4.5f;
                    break;
                
                case '2': 
                    current_focus = 5.0f;
                    break;

                case '3':
                    current_focus = 5.5f;
                    break;
                
                case '4':
                    current_focus = 6.0f;
                    break;
                
                case '5':
                    current_focus = 6.5f;
                    break;
                
                default:
                    break;
            }
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

    glClearAccum(0.0f, 0.0f, 0.0f, 0.0);

    //set up light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    //enable lighting 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //teapot display list
    teapotList = glGenLists(1);
    glNewList(teapotList, GL_COMPILE);
        glBegin(GL_TRIANGLES);
            for(int i = 0; i < sizeof(face_indicies) / sizeof(face_indicies[0]); i++)
            {
                for(int j = 0; j < 3; j++)
                {
                    int vi = face_indicies[i][j];
                    int ni = face_indicies[i][j + 3];

                    glNormal3f(normals[ni][0], normals[ni][1], normals[ni][2]);
                    glVertex3f(vertices[vi][0], vertices[vi][1], vertices[vi][2]);
                }
            }
        glEnd();
    glEndList();

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
    void renderTeapot(GLfloat x, GLfloat y, GLfloat z, 
                    GLfloat ambr, GLfloat ambg, GLfloat ambb,
                    GLfloat difr, GLfloat difg, GLfloat difb,
                    GLfloat specr, GLfloat specg, GLfloat specb,
                    GLfloat shine);
    void accPerspective(GLdouble fovy, GLdouble aspect, 
                    GLdouble zNear, GLdouble zFar, 
                    GLdouble pixdx, GLdouble pixdy,
                    GLdouble eyedx, GLdouble eyedy,
                    GLdouble focus);

    //variable declaration
    int jitter;
    GLint viewport[4];

    //code
    glGetIntegerv(GL_VIEWPORT, viewport);
    glClear(GL_ACCUM_BUFFER_BIT);

    for(jitter = 0; jitter < 8; jitter++)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        accPerspective(45.0f, (GLdouble)viewport[2] / (GLdouble)viewport[3], 
                       1.0f, 15.0f, 0.0f, 0.0f,
                       0.3f * j8[jitter].x, 0.3f * j8[jitter].y, current_focus);

        //ruby
        renderTeapot(-1.1f, -0.5f, -4.5f, 
                     0.1745f, 0.01175f, 0.01175f,
                     0.61424f, 0.04136f, 0.04136f,
                     0.727811f, 0.626959f, 0.626959f, 
                     0.6f);

        //gold
        renderTeapot(-0.5f, -0.5f, -5.0f, 
                     0.24725f, 0.1995f, 0.0745f,
                     0.75164f, 0.60648f, 0.22648f,
                     0.628281f, 0.555802f, 0.366065f,
                     0.4f);
        
        //silver 
        renderTeapot(0.2f, -0.5f, -5.5f, 
                     0.19225f, 0.19225f, 0.19225f,
                     0.50754f, 0.50754f, 0.50754f,
                     0.508273f, 0.508273f, 0.508273f,
                     0.4f);
        
        //emrald
        renderTeapot(1.0f, -0.5f, -6.0f,
                     0.0215f, 0.1745f, 0.0215f,
                     0.07568f, 0.61424f, 0.07568f,
                     0.633f, 0.727811f, 0.633f,
                     0.6f);
                
        //cyan
        renderTeapot(1.8f, -0.5f, -6.5f, 
                     0.0f, 0.1f, 0.06f, 
                     0.0f, 0.50980392f, 0.50980392f,
                     0.50196078f, 0.50196078f, 0.50196078f,
                     0.25f);

        glAccum(GL_ACCUM, 0.125f);
    }    
    glAccum(GL_RETURN, 1.0f);

    SwapBuffers(ghdc);
}

void renderTeapot(GLfloat x, GLfloat y, GLfloat z, 
                  GLfloat ambr, GLfloat ambg, GLfloat ambb,
                  GLfloat difr, GLfloat difg, GLfloat difb,
                  GLfloat specr, GLfloat specg, GLfloat specb,
                  GLfloat shine)
{
    //variable declaration
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];

    //code
    glPushMatrix();
        glTranslatef(x, y, z);

        materialAmbient[0] = ambr;
        materialAmbient[1] = ambg;
        materialAmbient[2] = ambb;
        materialAmbient[3] = 1.0f;

        materialDiffuse[0] = difr;
        materialDiffuse[1] = difg;
        materialDiffuse[2] = difb;
        materialDiffuse[3] = 1.0f;

        materialSpecular[0] = specr;
        materialSpecular[1] = specg;
        materialSpecular[2] = specb;
        materialSpecular[3] = 1.0f;

        //set up material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, shine * 128.0f);

        glCallList(teapotList);
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

    glDeleteLists(teapotList, 1);

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
