#include <gtest/gtest.h>
#include "StringHashMapTests.h"
#include <cstring>
#include <cstdlib>

void ParseCmdLineArgs(int argc, char** argv)
{
    for(int i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--hashmap-fuzz-seed") == 0)
        {
            if(i + 1 < argc)
            {
                gOverrideFuzzTestSeed = true;
                gFuzzTestSeed = strtoul(argv[i + 1], NULL, 10);
            }
        }
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest();
    ParseCmdLineArgs(argc, argv);
    return RUN_ALL_TESTS();
}