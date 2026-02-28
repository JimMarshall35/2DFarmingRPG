#ifndef WFCMDARGS_H
#define WFCMDARGS_H

struct WfGameCmdArgs
{
    const char* savesDir;
    const char* assetsDir;
};

extern struct WfGameCmdArgs gGameArgs;

#endif