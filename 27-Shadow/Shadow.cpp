// ------------------------
// Name :        Yash Patel
// Assignment :  Shadow Assignment 
// Date :        29-11-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Shadow.h"

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

GLuint groundList;                                       //ground display list
GLuint ground_texture;                                   //ground texture
GLuint treeList;                                         //tree display list

GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightAmbient[]  = {0.1f, 0.1f, 0.1f, 1.0f};
GLfloat lightDiffuse[]  = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat lightSpecular[] = {0.7f, 0.7f, 0.7f, 1.0f}; 

GLfloat shadowMat[16];

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
        TEXT("OpenGL : Shadow"),
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
    bool loadGLTexture(GLuint *texture, TCHAR ResourceID[]);
    void RenderGround(void);
    void RenderTree(bool bShadow);
    void ShadowMatrix(float *proj, const float *planeEq, const float *lightPos);
    void GetPlaneEquation(float *planeEq, const float *p1, const float *p2, const float *p3);

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
    float point1[] = {0.0f, -0.4f, 0.0f};
    float point2[] = {10.0f, -0.4f, 0.0f};
    float point3[] = {5.0f, -0.4f, -5.0f};
    float planeEquation[4];

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //back face culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //enable lighting and material properties
    glEnable(GL_LIGHTING);

    //set up light 1
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT1);

    //enable color tracking
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    //enable texture memory
    glEnable(GL_TEXTURE_2D);

    //load textures
    loadGLTexture(&ground_texture, MAKEINTRESOURCE(GROUND_BITMAP));

    //create and initialize display lists
    groundList = glGenLists(1);
    glNewList(groundList, GL_COMPILE);
        RenderGround();
    glEndList();

    treeList = glGenLists(1);
    glNewList(treeList, GL_COMPILE);
        RenderTree(false);
    glEndList();

    //construct shadow matrix
    GetPlaneEquation(planeEquation, point1, point2, point3);
    ShadowMatrix(shadowMat, planeEquation, lightPosition);

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

// --- loadGLTexture() - loads texture from resource ---
bool loadGLTexture(GLuint *texture, TCHAR ResourceID[])
{
    //variable declaration
    bool bResult = false;
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    //code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitmap)
    {
        bResult = true;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        //generate texture object
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        //setting texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //push the data to texture memory
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
    
        //free bitmap object
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return (bResult);
}

void RenderGround(void)
{
    GLfloat fExtent = 20.0f;
    GLfloat fStep = 1.0f;
    GLfloat y = -0.4f;
    GLfloat iStrip, iRun;
    GLfloat s = 0.0f;
    GLfloat t = 0.0f;
    GLfloat texStep = 1.0f / (fExtent * 0.055f);

    glBindTexture(GL_TEXTURE_2D, ground_texture);
    glColor3f(1.0f, 1.0f, 1.0f);
    for(iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        t = 0.0f;
        glBegin(GL_TRIANGLE_STRIP);
            for(iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
            {
                glNormal3f(0.0f, 1.0f, 0.0f);

                glTexCoord2f(s, t);
                glVertex3f(iStrip, y, iRun);

                glTexCoord2f(s + texStep, t);
                glVertex3f(iStrip + fStep, y, iRun);

                t += texStep;
            }
        glEnd();
        s += texStep;
    }
}

void RenderTree(bool bShadow)
{
    //variable declaration
    GLUquadric *quadric = NULL;

    //code
    glBindTexture(GL_TEXTURE_2D, NULL);
    glColor3f(0.0f, 0.0f, 0.0f);

    glPushMatrix();
        //main branch
        glTranslatef(0.0f, 0.1f, 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);

        if(!bShadow)
            glColor3f(0.55294f, 0.38039f, 0.258823f);
        
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.2f, 0.25f, 1.0f, 6, 2);
        
        //leaves
        if(!bShadow)
            glColor3f(0.0f, 0.333333f, 0.0f);
        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.7f, 1.0f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.6f, 0.9f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.5f, 0.8f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.4f, 0.7f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.3f, 0.6f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -1.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.0f, 0.5f, 1.2f, 10, 2);
    glPopMatrix();

    gluDeleteQuadric(quadric);
    quadric = NULL;
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
    //variable declaration
    static float speed = 0.0f;
    float x = 10.0f * cos(speed);
    float z = 10.0f * sin(speed);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt((GLfloat)x, 0.0f, (GLfloat)z, 
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    glPushMatrix();
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
        
        glCallList(groundList);

        //draw shadow first
        glPushMatrix();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            
            glMultMatrixf(shadowMat);
            RenderTree(true);

            glEnable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        
        glCallList(treeList);
    glPopMatrix();
    
    //update 
    speed += 0.001f;
    if(speed >= 360.0f)
        speed = 0.0f;

    SwapBuffers(ghdc);
}

void ShadowMatrix(float *proj, const float *planeEq, const float *lightPos)
{
    //variable declaration
    float a = planeEq[0];
    float b = planeEq[1];
    float c = planeEq[2];
    float d = planeEq[3];

    float dx = -lightPos[0];
    float dy = -lightPos[1];
    float dz = -lightPos[2];

    //code
    //fill the projction matrix
    proj[0] = b * dy + c * dz;
    proj[1] = -a * dy;
    proj[2] = -a * dz;
    proj[3] = 0.0f;

    proj[4] = -b * dx;
    proj[5] = a * dx + c * dz;
    proj[6] = -b * dz;
    proj[7] = 0.0f;

    proj[8] = -c * dx;
    proj[9] = -c * dy;
    proj[10] = a * dx + b * dy;
    proj[11] = 0.0f;

    proj[12] = -d * dx;
    proj[13] = -d * dy;
    proj[14] = -d * dz;
    proj[15] = a * dx + b * dy + c * dz;
}

void GetPlaneEquation(float *planeEq, const float *p1, const float *p2, const float *p3)
{
    //function declaration
    void CrossProduct(const float vector1[], const float vector2[], float out[3]);

    //variable declaration
    float vec1[3];
    float vec2[3];

    //code
    //vector1 = point3 - point1
    vec1[0] = p3[0] - p1[0];
    vec1[1] = p3[1] - p1[1];
    vec1[2] = p3[2] - p1[2];

    //vector2 = point2 - point1
    vec2[0] = p2[0] - p1[0];
    vec2[1] = p2[1] - p1[1];
    vec2[2] = p2[2] - p1[2];

    //unit normal to plane
    CrossProduct(vec1, vec2, planeEq);

    //substitute any of point to get value of constat d
    planeEq[3] = -(planeEq[0] * p3[0] + planeEq[1] * p3[1] + planeEq[2] * p3[2]);
}

void CrossProduct(const float vector1[], const float vector2[], float out[3])
{
    //function declaration 
    void Normalize(float vector[]);

    //code
    out[0] = vector1[1] * vector2[2] - vector1[2] * vector2[1];
    out[1] = vector1[2] * vector2[0] - vector1[0] * vector2[2];
    out[2] = vector1[0] * vector2[1] - vector1[1] * vector2[0];

    Normalize(out);
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

    //delete display lists
    glDeleteLists(groundList, 1);
    glDeleteLists(treeList, 1);

    //delete textures
    glDeleteTextures(1, &ground_texture);

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
