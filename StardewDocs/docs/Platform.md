# Platform

Platform.c / Platform.h abstract the creation of a window and opengl context, and handling of input (passing basic values to the engine input system).

Two back ends can be selected, GLFW3 or SDL2 at build time with the `STARDEW_PLATFORM` flag (see [Build](./Build.md)).