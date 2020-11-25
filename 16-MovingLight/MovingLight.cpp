// ------------------------
// Name :        Yash Patel
// Assignment :  Moving Light Assignment 
// Date :        24-11-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "MovingLight.h"

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

GLUquadric *quadric     = NULL;
float spin;

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
        TEXT("OpenGL : Moving Light"),
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
                
                case VK_LEFT:
                    spin -= 1.0f;
                    if(spin <= 0.0f)
                        spin = 360.0f;
                    break;
                
                case VK_RIGHT:
                    spin += 1.0f;
                    if(spin >= 360.0f)
                        spin = 0.0f;
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
    GLfloat lightModelAmbient[]   = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat whiteLight[]          = {1.0f, 1.0f, 1.0f, 1.0f};

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

    //set up light 0
    glLightModelfv(GL_AMBIENT_AND_DIFFUSE, lightModelAmbient);
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
    void DrawHelix(float radius, int twists);

    //variable declaration
    GLfloat lightPos0[] = {3.0f, 1.0f, 0.0f, 1.0f};

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0.0f, 0.0f, 10.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    glPushMatrix();
        glRotatef((GLfloat)spin, 0.0f, 1.0f, 0.0f);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

        glDisable(GL_LIGHTING);
        
        glTranslatef(lightPos0[0], lightPos0[1], lightPos0[2]);
        glColor3f(1.0f, 1.0f, 0.0f);
        quadric = gluNewQuadric();
        gluSphere(quadric, 0.3f, 10, 10);
        
        glEnable(GL_LIGHTING);
    glPopMatrix();

    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    DrawHelix(0.5f, 2);

    SwapBuffers(ghdc);
}

// --- DrawHelix() - renders spring ---
void DrawHelix(float radius, int twists)
{
    //function declaration
    void FindNormal(float vertices[3][3], float outNormals[3]);

    //variable declaration
    GLfloat x, y, z;
    GLfloat phi, theta, thetaRad, phiRad;
    GLfloat vertices[4][3];
    GLfloat normal[3];

    GLfloat matAmbient[]  = {0.23125f, 0.23125f, 0.23125f, 1.0f};;
    GLfloat matDiffuse[]  = {0.2775f, 0.2775f, 0.2775f, 1.0f};
    GLfloat matSpecular[] = {0.773911f, 0.773911f, 0.773911f, 1.0f};
    GLfloat matShininess  = 89.6f;

    //code
    glPushMatrix();
        //material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, matShininess);

        glBegin(GL_QUADS);
        for(phi = 0.0f; phi <= 360.0f; phi += 5.0f)
        {
            for(theta = 0.0f; theta <= 360.0f * twists; theta += 5.0f)
            {
                //first vertex
                phiRad = phi * (M_PI / 180.0f);
                thetaRad = theta * (M_PI / 180.0f);

                x = radius * cosf(thetaRad) * (2.0f + cosf(phiRad));
                y = radius * sinf(thetaRad) * (2.0f + cosf(phiRad));
                z = radius * ((thetaRad - (2.0f * M_PI))) + sinf(phiRad);

                vertices[0][0] = x;
                vertices[0][1] = y;
                vertices[0][2] = z;

                //second vertex
                phiRad = phi * (M_PI / 180.0f);
                thetaRad = (theta + 5.0f) * (M_PI / 180.0f);

                x = radius * cosf(thetaRad) * (2.0f + cosf(phiRad));
                y = radius * sinf(thetaRad) * (2.0f + cosf(phiRad));
                z = radius * ((thetaRad - (2.0f * M_PI))) + sinf(phiRad);

                vertices[1][0] = x;
                vertices[1][1] = y;
                vertices[1][2] = z;

                //third vertex
                phiRad = (phi + 5.0f) * (M_PI / 180.0f);
                thetaRad = (theta + 5.0f) * (M_PI / 180.0f);

                x = radius * cosf(thetaRad) * (2.0f + cosf(phiRad));
                y = radius * sinf(thetaRad) * (2.0f + cosf(phiRad));
                z = radius * ((thetaRad - (2.0f * M_PI))) + sinf(phiRad);

                vertices[2][0] = x;
                vertices[2][1] = y;
                vertices[2][2] = z;

                //forth vertex
                phiRad = (phi + 5.0f) * (M_PI / 180.0f);
                thetaRad = theta * (M_PI / 180.0f);

                x = radius * cosf(thetaRad) * (2.0f + cosf(phiRad));
                y = radius * sinf(thetaRad) * (2.0f + cosf(phiRad));
                z = radius * ((thetaRad - (2.0f * M_PI))) + sinf(phiRad);

                vertices[3][0] = x;
                vertices[3][1] = y;
                vertices[3][2] = z;

                //calculate normals
                FindNormal(vertices, normal);

                //render the quad
                glNormal3f(normal[0], normal[1], normal[2]);

                glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);   
                glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
                glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
                glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);         
            }
        }
        glEnd();

    glPopMatrix();

}

// --- FindNormal() - calculates normal using 3 points ---
void FindNormal(float vertices[3][3], float outNormal[3])
{
    //function declaration
    void ScaleVector(float vector[3]);

    //variable declaration
    float vec1[3], vec2[3];                
    static const int x = 0;
    static const int y = 1;
    static const int z = 2;

    //code
    //calculate vector from point 1 and 0
    vec1[0] = vertices[0][x] - vertices[1][x];
    vec1[1] = vertices[0][y] - vertices[1][y];
    vec1[2] = vertices[0][z] - vertices[1][z];

    //calculate vector from point 2 and 1
    vec2[0] = vertices[1][x] - vertices[2][x];
    vec2[1] = vertices[1][y] - vertices[2][y];
    vec2[2] = vertices[1][z] - vertices[2][z];

    //compute cross product 
    outNormal[x] = vec1[y] * vec2[z] - vec1[z] * vec2[y];
    outNormal[y] = vec1[z] * vec2[x] - vec1[x] * vec2[z];
    outNormal[z] = vec1[x] * vec2[y] - vec1[y] * vec2[x];

    ScaleVector(outNormal);
}

// --- ScaleVector() - reduces vector to unit length ---
void ScaleVector(float vector[3])
{
    //code
    float length = sqrtf((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));

    if(length == 0.0f)
        length = 1.0f;

    vector[0] = vector[0] / length;
    vector[1] = vector[1] / length;
    vector[2] = vector[2] / length;
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
