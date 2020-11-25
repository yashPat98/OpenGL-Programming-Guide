// ------------------------
// Name :        Yash Patel
// Assignment :  Material Assignment 
// Date :        25-11-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Material.h"

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// --- Macros ---
#define WIN_WIDTH  800                                   //window width
#define WIN_HEIGHT 600                                   //window height

#define X          0.525731112119133606f                 //from redbook
#define Z          0.850650808352039932f                 //from redbook 

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

GLfloat noMat[]        = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat matAmbient[]    = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat matAmbientCol[] = {0.8f, 0.8f, 0.2f, 1.0f};
GLfloat matDiffuse[]    = {0.1f, 0.5f, 0.8f, 1.0f};
GLfloat matSpecular[]   = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat matEmission[]   = {0.3f, 0.2f, 0.2f, 1.0f};
GLfloat noShininess     = 0.0f;
GLfloat lowShininess    = 5.0f;
GLfloat highShininess   = 100.0f;

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
        TEXT("OpenGL : Material"),
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
                
                case 49:
                    {
                        glMaterialfv(GL_FRONT, GL_AMBIENT, noMat);
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
                        glMaterialfv(GL_FRONT, GL_SPECULAR, noMat);
                        glMaterialf(GL_FRONT, GL_SHININESS, noShininess);
                        glMaterialfv(GL_FRONT, GL_EMISSION, noMat);
                    }
                    break;

                case 50:
                    {
                        glMaterialfv(GL_FRONT, GL_AMBIENT, noMat);
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
                        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
                        glMaterialf(GL_FRONT, GL_SHININESS, lowShininess);
                        glMaterialfv(GL_FRONT, GL_EMISSION, noMat);
                    }
                    break;                    

                case 51:
                    {
                        glMaterialfv(GL_FRONT, GL_AMBIENT, noMat);
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
                        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
                        glMaterialf(GL_FRONT, GL_SHININESS, highShininess);
                        glMaterialfv(GL_FRONT, GL_EMISSION, noMat);
                    }
                    break;                

                case 52:
                    {
                        glMaterialfv(GL_FRONT, GL_AMBIENT, noMat);
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
                        glMaterialfv(GL_FRONT, GL_SPECULAR, noMat);
                        glMaterialf(GL_FRONT, GL_SHININESS, noShininess);
                        glMaterialfv(GL_FRONT, GL_EMISSION, matEmission);
                    }
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

    //variable declaration
    GLfloat lightPos[]            = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat whiteLight[]          = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightModelAmbient[]   = {0.1f, 0.1f, 0.1f, 1.0f};

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //enable lighting and material properties
    glEnable(GL_LIGHTING);

    //set up light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteLight);
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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

// --- Display() - renders scene ---
void Display(void)
{
    //function declaration
    void RenderIcosaheron(int div);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0.0f, 0.0f, 5.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    RenderIcosaheron(5);

    SwapBuffers(ghdc);
}

void RenderIcosaheron(int div)
{
    //function declaration
    void SubDivide(float *vec1, float *vec2, float *vec3, int depth);

    //variable declaration
    GLfloat vec1[3], vec2[3], norm[3];
    int i, j;

    //verices
    static GLfloat vdata[12][3] = {
        {-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
        {0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
        {Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
    };

    //indices
    static GLuint tindices[20][3] = {
        {1, 4, 0}, {4, 9, 0}, {4, 5, 9}, {8, 5, 4}, {1, 8, 4},
        {1, 10, 8}, {10, 3, 8}, {8, 3, 5}, {3, 2, 5}, {3, 7, 2},
        {3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11}, {6, 1, 0},
        {10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9}, {11, 2, 7}
    };

    //code
    for(i = 0; i < 20; i++)
    {
        SubDivide(&vdata[tindices[i][0]][0], 
                  &vdata[tindices[i][1]][0],
                  &vdata[tindices[i][2]][0], div);
    }
}

void SubDivide(float *vec1, float *vec2, float *vec3, int depth)
{
    //function declaration 
    void Normalize(float vector[]);
    void DrawTriangle(float *vec1, float *vec2, float *vec3);

    //variable declarations
    GLfloat vec12[3], vec23[3], vec31[3];
    int i;

    //code
    if(depth == 0)
    {
        DrawTriangle(vec1, vec2, vec3);
        return;
    }

    for(i = 0; i < 3; i++)
    {
        vec12[i] = (vec1[i] + vec2[i]) / 2.0f;
        vec23[i] = (vec2[i] + vec3[i]) / 2.0f;
        vec31[i] = (vec3[i] + vec1[i]) / 2.0f;
    }

    Normalize(vec12);
    Normalize(vec23);
    Normalize(vec31);

    SubDivide(vec1, vec12, vec31, depth - 1);
    SubDivide(vec2, vec23, vec12, depth - 1);
    SubDivide(vec3, vec31, vec23, depth - 1);
    SubDivide(vec12, vec23, vec31, depth - 1);
}

void DrawTriangle(float *vec1, float *vec2, float *vec3)
{
    //code
    glBegin(GL_TRIANGLES);
        glNormal3fv(vec1);
        glVertex3fv(vec1);
        glNormal3fv(vec2);
        glVertex3fv(vec2);
        glNormal3fv(vec3);
        glVertex3fv(vec3);
    glEnd();
}

void Normalize(float vector[])
{
    //code
    GLfloat dist = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

    if(dist == 0.0f)
        return;
    
    vector[0] = vector[0] / dist;
    vector[1] = vector[1] / dist;
    vector[2] = vector[2] / dist;
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
