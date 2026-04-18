#include "Platform.h"

#include "main.h"
#include "InputContext.h"
#include "PlatformDefs.h"
#include "KeyCodes.h"

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
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

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
#include <glad/glad.h>

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
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    // Request version 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    

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
    
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    Log_Verbose("loading Opengl procs\n");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
    Log_Verbose("loading Opengl ES procs");
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#endif


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

static int SDLScancodeToStardewKey(SDL_Scancode sc)
{
    switch (sc)
    {
        /* Printable keys */
        case SDL_SCANCODE_SPACE: return STARDEW_KEY_SPACE;
        case SDL_SCANCODE_APOSTROPHE: return STARDEW_KEY_APOSTROPHE;
        case SDL_SCANCODE_COMMA: return STARDEW_KEY_COMMA;
        case SDL_SCANCODE_MINUS: return STARDEW_KEY_MINUS;
        case SDL_SCANCODE_PERIOD: return STARDEW_KEY_PERIOD;
        case SDL_SCANCODE_SLASH: return STARDEW_KEY_SLASH;

        case SDL_SCANCODE_0: return STARDEW_KEY_0;
        case SDL_SCANCODE_1: return STARDEW_KEY_1;
        case SDL_SCANCODE_2: return STARDEW_KEY_2;
        case SDL_SCANCODE_3: return STARDEW_KEY_3;
        case SDL_SCANCODE_4: return STARDEW_KEY_4;
        case SDL_SCANCODE_5: return STARDEW_KEY_5;
        case SDL_SCANCODE_6: return STARDEW_KEY_6;
        case SDL_SCANCODE_7: return STARDEW_KEY_7;
        case SDL_SCANCODE_8: return STARDEW_KEY_8;
        case SDL_SCANCODE_9: return STARDEW_KEY_9;

        case SDL_SCANCODE_SEMICOLON: return STARDEW_KEY_SEMICOLON;
        case SDL_SCANCODE_EQUALS: return STARDEW_KEY_EQUAL;
        case SDL_SCANCODE_GRAVE: return STARDEW_KEY_GRAVE_ACCENT;

        case SDL_SCANCODE_LEFTBRACKET: return STARDEW_KEY_LEFT_BRACKET;
        case SDL_SCANCODE_BACKSLASH: return STARDEW_KEY_BACKSLASH;
        case SDL_SCANCODE_RIGHTBRACKET: return STARDEW_KEY_RIGHT_BRACKET;

        /* Letters */
        case SDL_SCANCODE_A: return STARDEW_KEY_A;
        case SDL_SCANCODE_B: return STARDEW_KEY_B;
        case SDL_SCANCODE_C: return STARDEW_KEY_C;
        case SDL_SCANCODE_D: return STARDEW_KEY_D;
        case SDL_SCANCODE_E: return STARDEW_KEY_E;
        case SDL_SCANCODE_F: return STARDEW_KEY_F;
        case SDL_SCANCODE_G: return STARDEW_KEY_G;
        case SDL_SCANCODE_H: return STARDEW_KEY_H;
        case SDL_SCANCODE_I: return STARDEW_KEY_I;
        case SDL_SCANCODE_J: return STARDEW_KEY_J;
        case SDL_SCANCODE_K: return STARDEW_KEY_K;
        case SDL_SCANCODE_L: return STARDEW_KEY_L;
        case SDL_SCANCODE_M: return STARDEW_KEY_M;
        case SDL_SCANCODE_N: return STARDEW_KEY_N;
        case SDL_SCANCODE_O: return STARDEW_KEY_O;
        case SDL_SCANCODE_P: return STARDEW_KEY_P;
        case SDL_SCANCODE_Q: return STARDEW_KEY_Q;
        case SDL_SCANCODE_R: return STARDEW_KEY_R;
        case SDL_SCANCODE_S: return STARDEW_KEY_S;
        case SDL_SCANCODE_T: return STARDEW_KEY_T;
        case SDL_SCANCODE_U: return STARDEW_KEY_U;
        case SDL_SCANCODE_V: return STARDEW_KEY_V;
        case SDL_SCANCODE_W: return STARDEW_KEY_W;
        case SDL_SCANCODE_X: return STARDEW_KEY_X;
        case SDL_SCANCODE_Y: return STARDEW_KEY_Y;
        case SDL_SCANCODE_Z: return STARDEW_KEY_Z;

        /* Controls */
        case SDL_SCANCODE_ESCAPE: return STARDEW_KEY_ESCAPE;
        case SDL_SCANCODE_RETURN: return STARDEW_KEY_ENTER;
        case SDL_SCANCODE_TAB: return STARDEW_KEY_TAB;
        case SDL_SCANCODE_BACKSPACE: return STARDEW_KEY_BACKSPACE;
        case SDL_SCANCODE_INSERT: return STARDEW_KEY_INSERT;
        case SDL_SCANCODE_DELETE: return STARDEW_KEY_DELETE;

        case SDL_SCANCODE_RIGHT: return STARDEW_KEY_RIGHT;
        case SDL_SCANCODE_LEFT: return STARDEW_KEY_LEFT;
        case SDL_SCANCODE_DOWN: return STARDEW_KEY_DOWN;
        case SDL_SCANCODE_UP: return STARDEW_KEY_UP;

        case SDL_SCANCODE_PAGEUP: return STARDEW_KEY_PAGE_UP;
        case SDL_SCANCODE_PAGEDOWN: return STARDEW_KEY_PAGE_DOWN;
        case SDL_SCANCODE_HOME: return STARDEW_KEY_HOME;
        case SDL_SCANCODE_END: return STARDEW_KEY_END;

        case SDL_SCANCODE_CAPSLOCK: return STARDEW_KEY_CAPS_LOCK;
        case SDL_SCANCODE_SCROLLLOCK: return STARDEW_KEY_SCROLL_LOCK;
        case SDL_SCANCODE_NUMLOCKCLEAR: return STARDEW_KEY_NUM_LOCK;
        case SDL_SCANCODE_PRINTSCREEN: return STARDEW_KEY_PRINT_SCREEN;
        case SDL_SCANCODE_PAUSE: return STARDEW_KEY_PAUSE;

        /* Function keys */
        case SDL_SCANCODE_F1: return STARDEW_KEY_F1;
        case SDL_SCANCODE_F2: return STARDEW_KEY_F2;
        case SDL_SCANCODE_F3: return STARDEW_KEY_F3;
        case SDL_SCANCODE_F4: return STARDEW_KEY_F4;
        case SDL_SCANCODE_F5: return STARDEW_KEY_F5;
        case SDL_SCANCODE_F6: return STARDEW_KEY_F6;
        case SDL_SCANCODE_F7: return STARDEW_KEY_F7;
        case SDL_SCANCODE_F8: return STARDEW_KEY_F8;
        case SDL_SCANCODE_F9: return STARDEW_KEY_F9;
        case SDL_SCANCODE_F10: return STARDEW_KEY_F10;
        case SDL_SCANCODE_F11: return STARDEW_KEY_F11;
        case SDL_SCANCODE_F12: return STARDEW_KEY_F12;

        /* Keypad */
        case SDL_SCANCODE_KP_0: return STARDEW_KEY_KP_0;
        case SDL_SCANCODE_KP_1: return STARDEW_KEY_KP_1;
        case SDL_SCANCODE_KP_2: return STARDEW_KEY_KP_2;
        case SDL_SCANCODE_KP_3: return STARDEW_KEY_KP_3;
        case SDL_SCANCODE_KP_4: return STARDEW_KEY_KP_4;
        case SDL_SCANCODE_KP_5: return STARDEW_KEY_KP_5;
        case SDL_SCANCODE_KP_6: return STARDEW_KEY_KP_6;
        case SDL_SCANCODE_KP_7: return STARDEW_KEY_KP_7;
        case SDL_SCANCODE_KP_8: return STARDEW_KEY_KP_8;
        case SDL_SCANCODE_KP_9: return STARDEW_KEY_KP_9;

        case SDL_SCANCODE_KP_DECIMAL: return STARDEW_KEY_KP_DECIMAL;
        case SDL_SCANCODE_KP_DIVIDE: return STARDEW_KEY_KP_DIVIDE;
        case SDL_SCANCODE_KP_MULTIPLY: return STARDEW_KEY_KP_MULTIPLY;
        case SDL_SCANCODE_KP_MINUS: return STARDEW_KEY_KP_SUBTRACT;
        case SDL_SCANCODE_KP_PLUS: return STARDEW_KEY_KP_ADD;
        case SDL_SCANCODE_KP_ENTER: return STARDEW_KEY_KP_ENTER;
        case SDL_SCANCODE_KP_EQUALS: return STARDEW_KEY_KP_EQUAL;

        /* Modifiers */
        case SDL_SCANCODE_LSHIFT: return STARDEW_KEY_LEFT_SHIFT;
        case SDL_SCANCODE_LCTRL: return STARDEW_KEY_LEFT_CONTROL;
        case SDL_SCANCODE_LALT: return STARDEW_KEY_LEFT_ALT;
        case SDL_SCANCODE_LGUI: return STARDEW_KEY_LEFT_SUPER;

        case SDL_SCANCODE_RSHIFT: return STARDEW_KEY_RIGHT_SHIFT;
        case SDL_SCANCODE_RCTRL: return STARDEW_KEY_RIGHT_CONTROL;
        case SDL_SCANCODE_RALT: return STARDEW_KEY_RIGHT_ALT;
        case SDL_SCANCODE_RGUI: return STARDEW_KEY_RIGHT_SUPER;

        case SDL_SCANCODE_MENU: return STARDEW_KEY_MENU;

        default:
            return STARDEW_KEY_LAST;
    }
}

void Platform_PollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        
        switch(event.type)
        {
        case SDL_QUIT:
            gShouldWIndowClose = true;
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
        case SDL_KEYDOWN:
            {
                int sdKey = SDLScancodeToStardewKey(event.key.keysym.scancode);
                In_RecieveKeyboardKey(GetInputContext(), sdKey, event.key.keysym.scancode, STARDEW_PRESS, 0); // todo: mods
            }
            break;
        case SDL_KEYUP:
            {
                int sdKey = SDLScancodeToStardewKey(event.key.keysym.scancode);
                In_RecieveKeyboardKey(GetInputContext(), sdKey, event.key.keysym.scancode, STARDEW_RELEASE, 0); // todo: mods
            }
            break;
        case SDL_MOUSEMOTION:
            {
                int x = event.motion.x;
                int y = event.motion.y;
                In_RecieveMouseMove(GetInputContext(), (double)x, (double)y);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            In_RecieveMouseButton(GetInputContext(), event.button.button - 1, STARDEW_PRESS, 0); // sdl uses 1 based indices for the mouse buttons, glfw uses 0 based
            break;
        case SDL_MOUSEBUTTONUP:
            In_RecieveMouseButton(GetInputContext(), event.button.button - 1, STARDEW_RELEASE, 0);
            break;
        case SDL_MOUSEWHEEL:
            // may be handled differently to glfw, not tested
            In_RecieveScroll(GetInputContext(), (double)event.wheel.x, (double)event.wheel.y);
            break;
        }
    }
}


#endif

