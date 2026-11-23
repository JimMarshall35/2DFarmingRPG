#include "NetworkID.h"

static int gNetID = 0;

void NetID_DeserializedNewID(int nextVal)
{
    if((nextVal + 1) > gNetID)
        gNetID = nextVal + 1;
}

int NetID_GetID()
{
    return gNetID++;
}
