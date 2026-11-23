#include "BitField2D.h"
#include <string.h>
#include <stdlib.h>


struct Bitfield2D
{
    int w;
    int h;
    int sizeBytes;
    u8* pData;
};

struct Bitfield2D* Bf2D_NewBitField(int w, int h)
{
    struct Bitfield2D* pBF = malloc(sizeof(struct Bitfield2D));
    pBF->w = w;
    pBF->h = h;
    pBF->sizeBytes = (w * h) % 8 ? ((w * h) / 8) + 1 : (w * h) / 8;
    pBF->pData = malloc(pBF->sizeBytes);
    return pBF;
}

void Bf2D_FreeBitField(struct Bitfield2D* pBF)
{
    free(pBF->pData);
    free(pBF);
}

void Bf2D_ClearBitField(struct Bitfield2D* pBF)
{
    memset(pBF->pData, 0, pBF->sizeBytes);
}

bool Bf2D_IsBitSet(struct Bitfield2D* pBF, int x, int y)
{
    int i = y * pBF->w + x;
    u8 d = pBF->pData[i / 8];
    return d & (1 << (i % 8));
}

void Bf2D_SetBit(struct Bitfield2D* pBF, int x, int y, bool newVal)
{
    pBF->pData[(y * pBF->w + x) / 8] |= 1 << ((y * pBF->w + x) % 8);
}

void Bf2D_SetBitfieldRegion(struct Bitfield2D* pBF, int x, int y, int w, int h)
{
    int initialX = x;
    for(int i=0; i<h; i++)
    {
        for(int j=0; j< w; j++)
        {
            Bf2D_SetBit(pBF, x,y, true);
            ++x;
        }
        x = initialX;
        ++y;
    }
}


void Bf2D_ResizeAndClearBitField(struct Bitfield2D* pBF, int newW, int newH)
{
    free(pBF->pData);
    pBF->w = newW;
    pBF->h = newH;
    pBF->sizeBytes = (newW * newH) % 8 ? ((newW * newH) / 8) + 1 : (newW * newH) / 8;
    pBF->pData = malloc(pBF->sizeBytes);
}