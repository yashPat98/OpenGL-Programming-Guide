// ------------------------
// Name :        Yash Patel
// Assignment :  Complete Font using Display lists 
// Date :        01-12-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Font.h"

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

GLuint fontOffset;
GLubyte space[]         = {0x00, 0x00, 0x00, 0x00, 0x00, 
                           0x00, 0x00, 0x00, 0x00, 0x00, 
                           0x00, 0x00, 0x00}; 
GLubyte letters[][13]   = {{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18}, 
                           {0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
                           {0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
                           {0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc}, 
                           {0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff}, 
                           {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff}, 
                           {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e}, 
                           {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
                           {0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e}, 
                           {0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06}, 
                           {0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3}, 
                           {0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0}, 
                           {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3}, 
                           {0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3}, 
                           {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e}, 
                           {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
                           {0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c}, 
                           {0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe}, 
                           {0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e}, 
                           {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff}, 
                           {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
                           {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
                           {0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3}, 
                           {0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
                           {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3}, 
                           {0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff}};


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
        TEXT("OpenGL : Font"),
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
    void makeRasterFont(void);

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

    makeRasterFont();

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void makeRasterFont(void)
{
    //variable declaration
    GLuint i, j;

    //code
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    fontOffset = glGenLists(128);
    for(i = 0, j = 'A'; i < 26; i++, j++)
    {
        glNewList(fontOffset + j, GL_COMPILE);
            glBitmap(8, 13, 0.0f, 2.0f, 10.0f, 0.0f, letters[i]);
        glEndList();
    }

    glNewList(fontOffset + ' ', GL_COMPILE);
        glBitmap(8, 13, 0.0f, 2.0f, 10.0f ,0.0f, space);
    glEndList();
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

    glOrtho(0.0, (GLdouble)width, 0.0, (GLdouble)height, -1.0, 1.0);
}

// --- Display() - renders scene ---
void Display(void)
{
    //function declaration
    void printString(char *s);

    //code
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(20, 60);
    printString("THE QUICK BROWN FOX JUMPS");
    glRasterPos2i(20, 40);
    printString("OVER A LAZY DOG");

    SwapBuffers(ghdc);
}

void printString(char *s)
{
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontOffset);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
    glPopAttrib();
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
