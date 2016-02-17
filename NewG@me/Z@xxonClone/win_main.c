/* ---------------------------------------------------------------
   win_main.c
   Windows main for the game
   Based on CodeBlocks openGL template
   ---------------------------------------------------------------

   Copyright (c) 2013 by Marco Giorgini <marco.giorgini@gmail.com>
   This file is part of the New G@ame: Z@xxon (Loosy) Clone sample code.

   ---------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use,
   copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following
   conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   --------------------------------------------------------------
*/

#include <windows.h>
#include <gl/gl.h>

#include "g_main.h"

#ifdef SND_OPENAL
#pragma comment(lib, "al/openal32.lib")
#endif

#ifdef GFX_OPENGL
#pragma comment(lib, "opengl32.lib")
#endif

char szAppName[]="New G@me: Z@xxon Clone";
int  win_width=480*2,win_height=320*2;
int  win_framepersec=30;

char szWinClassName[]="marcogiorgini.com";

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    RECT rWindow;
    RECT rDesktop;
    BOOL bQuit = FALSE;
    DWORD dwStyle=WS_OVERLAPPEDWINDOW;
    char* strWindowName=szAppName;
    char  szResPath[]="";
    int   fit=1000/win_framepersec;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWinClassName;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    rWindow.left	= 0;
    rWindow.right	= win_width;
    rWindow.top	    = 0;
    rWindow.bottom	= win_height;

    AdjustWindowRect( &rWindow, dwStyle, 0);

    GetWindowRect(GetDesktopWindow(),&rDesktop);

    hwnd = CreateWindow(szWinClassName, strWindowName, dwStyle, (rDesktop.right-(rWindow.right  - rWindow.left))/2, (rDesktop.bottom-(rWindow.bottom - rWindow.top))/2,
                        rWindow.right  - rWindow.left, rWindow.bottom - rWindow.top,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    GAME_init(&g_W,szResPath,(float)win_width,(float)win_height);

    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            int it=os_getMilliseconds();

            GAME_loop(&g_W);

            SwapBuffers(hDC);

            it=os_getMilliseconds()-it;

            if(it<fit)
                Sleep(fit-it);
        }
    }

    GAME_reset(&g_W);

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

void setTOUCH(HWND hwnd,LPARAM lParam)
{
    if(os_portrait)
    {
        os_x[0]=LOWORD(lParam);
        os_y[0]=HIWORD(lParam);
    }
    else
    {
        RECT  g_rRect;
        GetClientRect(hwnd, &g_rRect);					// Get the window rectangle
        os_x[0]=g_rRect.bottom-HIWORD(lParam);
        os_y[0]=LOWORD(lParam);
    }
    if(os_flip)
    {
        RECT  g_rRect;
        GetClientRect(hwnd, &g_rRect);					// Get the window rectangle
        os_x[0]=g_rRect.left-os_x[0];
        os_y[0]=g_rRect.bottom-os_y[0];
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
        return 0;
    case WM_SIZE:										// If the window is resized
        os_video_w=LOWORD(lParam);
        os_video_h=HIWORD(lParam);
        glViewport(0,0,LOWORD(lParam),HIWORD(lParam));
        break;
    case WM_MOUSEMOVE:
        if(wParam&MK_LBUTTON)
            if(os_status[0])
            {
                os_np=1;
                os_status[0]=touchMOVE;
                setTOUCH(hwnd,lParam);
            }
        break;
    case WM_LBUTTONDOWN:
    {
        os_np=1;
        os_status[0]=touchDOWN;
        setTOUCH(hwnd,lParam);
    }
    break;
    case WM_LBUTTONUP:
    {
        os_np=1;
        os_status[0]=touchUP;
        setTOUCH(hwnd,lParam);
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

