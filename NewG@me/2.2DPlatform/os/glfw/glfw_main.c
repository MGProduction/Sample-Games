//========================================================================
// Simple multi-window test
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test creates four windows and clears each in a different color
//
//========================================================================

//#include <glad/glad.h>
#ifdef OS_WIN32
#include <windows.h>
#endif
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>

#include "g_main.h"

#ifdef SND_OPENAL
#pragma comment(lib, "lib/al/openal32.lib")
#endif

#ifdef GFX_OPENGL
#pragma comment(lib, "opengl32.lib")
#endif


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void cursor_pos_callback(GLFWwindow* window, double x,double y)
{
 int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
 if (state == GLFW_PRESS)
  {
   os_x[0]=x;
   os_y[0]=y;
   os_status[0]=touchMOVE;
   os_np=1;
  }
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
 int    k;
 double x,y;
 glfwGetCursorPos(window,&x,&y);
 if(action==GLFW_PRESS)
  {
   os_x[0]=x;
   os_y[0]=y;
   os_status[0]=touchDOWN;
   os_np=1;
  }
 else 
 if(action==GLFW_RELEASE)
  {
   os_x[0]=x;
   os_y[0]=y;
   os_status[0]=touchUP;
   os_np=1;
  }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    os_keys[key]=action;
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {


        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
    }
}

#if defined(OS_LINUX)
void S_getpath(const char*filename,char*path)
{
 int l=strlen(filename);
 if(filename!=path)
  strcpy(path,filename);
 while(l--)
  if((filename[l]!='/')&&(filename[l]!='\\'))
   ;
  else
  {
   path[l+1]=0;
   break;
  }
}
#endif

#ifdef OS_WIN32
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
    int running = GLFW_TRUE;
    char path[ 1024 ]="";
    int left, top, right, bottom;
    int fit=1000/30;
    GLFWwindow* windows;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // --- glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER,GLFW_TRUE);

    windows = glfwCreateWindow(320*2, 240*2, "2DWorld",  NULL, NULL);
    if (!windows)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(windows, key_callback);
    glfwSetCursorPosCallback(windows, cursor_pos_callback);
    glfwSetMouseButtonCallback(windows, mouse_callback);

    glfwMakeContextCurrent(windows);
    //gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glClearColor(0,0,0, 1.f);

    glfwGetWindowFrameSize(windows, &left, &top, &right, &bottom);
       /* if(0)
        glfwSetWindowPos(windows,
                         100 + (i & 1) * (200 + left + right),
                         100 + (i >> 1) * (200 + top + bottom));*/

    glfwShowWindow(windows);

    #if defined(OS_LINUX)
    readlink( "/proc/self/exe", path, 1024 );
    S_getpath(path,path);
    #endif
    if(!GAME_init(&g_W,path,320*2,240*2))
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (running)
    {
      int it=os_getMilliseconds(),err;

      glfwMakeContextCurrent(windows);
      glClear(GL_COLOR_BUFFER_BIT);

      GAME_loop(&g_W);
      err=glGetError();

      glfwSwapBuffers(windows);

      if (glfwWindowShouldClose(windows))
          running = GLFW_FALSE;

      while(os_getMilliseconds()-it<fit)
       glfwPollEvents();
    }

    GAME_reset(&g_W);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

