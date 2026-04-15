#include "Platform.h"

#include "main.h"
#include "InputContext.h"

void Common_FramebufferSizeChangeHandler(int width, int height)
{
    Dr_OnScreenDimsChange(GetDrawContext(), width, height);
    In_FramebufferResize(GetInputContext(), width, height);
    GF_OnWindowDimsChanged(width, height);

}

#if STARDEW_PLATFORM == STARDEW_PLATFORM_GLFW3

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLFWwindow* gWindow = NULL;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Common_FramebufferSizeChangeHandler(width, height);
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    In_RecieveMouseMove(GetInputContext(), xposIn, yposIn);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    In_RecieveScroll(GetInputContext(), xoffset, yoffset);
}

void MouseBtnCallback(GLFWwindow* window, int button, int action, int mods)
{
    In_RecieveMouseButton(GetInputContext(), button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    In_RecieveKeyboardKey(GetInputContext(), key, scancode, action, mods);
}

void joystick_callback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        In_SetControllerPresent(jid);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        In_SetControllerPresent(-1);
    }
}

int Platform_InitWindow()
{
     // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    Log_Verbose("glfwInit");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    const char* windowTitle = "Stardew Engine";
    switch(NW_GetRole())
    {
    case GR_Client:
        windowTitle = "Stardew Engine (Client)";
        break;
    case GR_ClientServer:
        windowTitle = "Stardew Engine (Server)";
        break;
    }
    gWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle, NULL, NULL);
    if (gWindow == NULL)
    {
        /*std::cout << "Failed to create GLFW window" << std::endl;*/
        Log_Error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(gWindow);
    glfwSwapInterval(0); // Enable vsync
    
    glfwJoystickPresent(GLFW_JOYSTICK_1);

    glfwSetFramebufferSizeCallback(gWindow, FramebufferSizeCallback);
    glfwSetCursorPosCallback(gWindow, MouseCallback);
    glfwSetScrollCallback(gWindow, ScrollCallback);
    glfwSetMouseButtonCallback(gWindow, MouseBtnCallback);
    glfwSetKeyCallback(gWindow, key_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    Log_Verbose("loading Opengl procs\n");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
    Log_Verbose("loading Opengl ES procs");
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#endif
    return 0;
}

bool Platform_ShouldWindowClose()
{
    return glfwWindowShouldClose(gWindow);
}

void Platform_SwapBuffers()
{
    glfwSwapBuffers(gWindow);
}

double Platform_GetElapsedSeconds()
{
    return glfwGetTime();
}

void Platform_DeInit()
{
    glfwTerminate();
}

void Platform_PollEvents()
{
    glfwPollEvents();
}

#elif STARDEW_PLATFORM == STARDEW_PLATFORM_SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include "Log.h"

SDL_Window* gWindow;
SDL_GLContext gOpenglContext = NULL;
bool gShouldWIndowClose = false;

int Platform_InitWindow()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        Log_Error("Failed to init SDL");
        return -1;
    }

    // Tell SDL we want OpenGL ES, not desktop GL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    // Request version 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // (Optional but usually desired)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    
    gWindow = SDL_CreateWindow("Stardew Engine SDL", 0, 0, SCR_WIDTH, SCR_HEIGHT,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(gWindow == NULL)
    {
        Log_Error("SDL_CreateWindow Failed");
        return -1;
    }
    gOpenglContext = SDL_GL_CreateContext(gWindow);
    if(gOpenglContext == NULL)
    {
        Log_Error("SDL_GL_CreateContext Failed");
        return -1;
    }


    return 0;
}

bool Platform_ShouldWindowClose()
{
    return gShouldWIndowClose;
}

void Platform_SwapBuffers()
{
    SDL_GL_SwapWindow(gWindow);
}

double Platform_GetElapsedSeconds()
{
    Uint32 ms = SDL_GetTicks();
    double seconds = ms / 1000.0;
    return seconds;
}

void Platform_DeInit()
{
    SDL_Quit();
}

void Platform_PollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        /* TODO: handle input */
        if (event.type == SDL_QUIT)
        {
            gShouldWIndowClose = false;
        }
        switch(event.type)
        {
        case SDL_QUIT:
            {
                gShouldWIndowClose = true;
            }
            break;
        case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    Common_FramebufferSizeChangeHandler(width, height);
                }
            }
            break;
        }
    }
}


#endif