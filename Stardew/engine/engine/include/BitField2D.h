#ifndef BITFIELD2D_H
#define BITFIELD2D_H
#include "IntTypes.h"
#include <stdbool.h>

struct Bitfield2D;

struct Bitfield2D* Bf2D_NewBitField(int w, int h);

void Bf2D_FreeBitField(struct Bitfield2D* pBF);

void Bf2D_ClearBitField(struct Bitfield2D* pBF);

bool Bf2D_IsBitSet(struct Bitfield2D* pBF, int x, int y);

void Bf2D_SetBit(struct Bitfield2D* pBF, int x, int y, bool newVal);

void Bf2D_SetBitfieldRegion(struct Bitfield2D* pBF, int x, int y, int w, int h);

void Bf2D_ResizeAndClearBitField(struct Bitfield2D* pBF, int newW, int newH);

#endif