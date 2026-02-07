#include "Audio.h"
#include "stdio.h"
#include "StardewString.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Log.h"

void Usage()
{
    const char* usage = 
    "zzfx cli:\n"
    "Play a sound generatd on https://killedbyapixel.github.io/ZzFX/ to see what it will sound like in the game"
    "[zzfx-args] [OPTIONS]\n"
    "zzfx-args:\n"
    "   the output from the inner function call generated from https://killedbyapixel.github.io/ZzFX/ when \"full\" is selected (defaults to \"compact\")"
    "   for example \"1.4,.05,61,.03,.27,.38,4,1.2,0,0,0,0,0,1.6,0,.4,.37,.35,.15,0,1256\""
    "OPTIONS:\n"
    "--master-vol -v\n"
    "   master volume, defaults to 0.2\n"
    "--repeats -r\n"
    "   repeat the sound effect this many times\n"
    "--wait-between-repeats -w\n"
    "   followed by the time to wait between repeats in seconds\n"
    ;
    printf(usage);
}

char* CopyString(const char* s)
{
    char* s2 = malloc(strlen(s) + 1);
    strcpy(s2, s);
    return s2;
}

struct ZZFXSound ParseZzfxCall(char* call)
{
    struct ZZFXSound snd;
    float* asFloats = &snd.volume;
    char* copy = CopyString(call);
    char* token = copy;
    int numTokens = Str_Tokenize(copy, ',');
    for(int i=0; i<numTokens; i++)
    {
        float f = (double)atof(token);
        asFloats[i] = f;
        Str_AdvanceToNextToken(&token);
    }
    free(copy);
    return snd;
}

int main(int argc, char** argv)
{
    int repeats = 1;
    float waitBetweenRepeats = 0.0f;
    float masterVol = 0.2f;
    if(argc <= 1)
    {
        Usage();
        return 1;
    }
    int onArg = 1;
    char* zzfxCall = argv[onArg++];
    struct ZZFXSound snd = ParseZzfxCall(zzfxCall);
    while(onArg < argc)
    {
        if(strcmp(argv[onArg], "--master-vol") == 0 || strcmp(argv[onArg], "-v") == 0)
        {
            onArg++;
            masterVol = (float)atof(argv[onArg]);
        }
        else if(strcmp(argv[onArg], "--repeats") == 0 || strcmp(argv[onArg], "-r") == 0)
        {
            onArg++;
            repeats = atoi(argv[onArg]);
        }
        else if(strcmp(argv[onArg], "--wait-between-repeats") == 0 || strcmp(argv[onArg], "-w") == 0)
        {
            onArg++;
            waitBetweenRepeats = (float)atof(argv[onArg]);
        }
        onArg++;
    }

    Log_Init();
    Au_Init();

    for(int i=0; i<repeats; i++)
    {
        float lengthSeconds = Au_PlaySoundTest(&snd, masterVol);
        usleep(lengthSeconds * 1000000 + waitBetweenRepeats * 1000000);
    }
    
    Au_DeInit();
    Log_DeInit();
}