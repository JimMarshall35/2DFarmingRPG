# Testing

There is a unit test project for the engine, in the Stardew/enginetest folder. This is to mainly test code in the engine/core folder although of course can test any engine code.

It's a C++ project, so to test new areas of the C code you'll have to add this to the engine headers of any file tested:

```c
#ifdef __cplusplus
extern "C" {
#endif


// header body...

#ifdef __cplusplus
}
#endif

```

In addition to the unit test project there's another integration level test project called enginenettest. This adds a new C executable that links to the engine to test the low level networking functionality (Network.c). This doesn't use any particular testing framework and is a custom made test. The test that uses this executable is the shell script EngineNetTest.sh.

One day I'll re-write EngineNetTest.sh in python so it can run in the CI on windows not just linux
