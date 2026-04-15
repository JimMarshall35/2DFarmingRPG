#include "Platform.h"

#include "main.h"
#include "InputContext.h"


#if STARDEW_PLATFORM == STARDEW_PLATFORM_GLFW3

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLFWwindow* gWindow = NULL;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Dr_OnScreenDimsChange(GetDrawContext(), width, height);
    In_FramebufferResize(GetInputContext(), width, height);
    GF_OnWindowDimsChanged(width, height);
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

int Platform_InitWindow()
{
    return -1;
}

bool Platform_ShouldWindowClose()
{
    return true;
}

void Platform_SwapBuffers()
{
}

double Platform_GetElapsedSeconds()
{
    return 0.0;
}

void Platform_DeInit()
{
}

void Platform_PollEvents()
{
}


#endif