#ifndef ATLAS_H
#define ATLAS_H

#include "IntTypes.h"
#include "HandleDefs.h"
#include <cglm/cglm.h>
#include <stdbool.h>

struct DrawContext;
typedef struct DrawContext DrawContext;
typedef struct _xmlNode xmlNode;
struct BinarySerializer;

#define VECTOR(a) a*

typedef struct _AtlasSprite
{
	hAtlas atlas;
	HImage srcImage;
	char* name;
	int srcImageTopLeftXPx;
	int srcImageTopLeftYPx;
	int widthPx;
	int heightPx;

	int atlasTopLeftXPx;
	int atlasTopLeftYPx;

	float topLeftUV_U;
	float topLeftUV_V;
	float bottomRightUV_U;
	float bottomRightUV_V;

	u8* individualTileBytes;
	bool bSet;

	int id;
}AtlasSprite;


#define MAX_FONT_NAME_SIZE 128
#define MAX_FONT_PATH_SIZE 256

#define MAX_NUM_FONT_SIZES 32

struct AtlasAnimation
{
	VECTOR(hSprite) frames;
	float fps;
};


struct FontSize
{
	enum
	{
		FOS_Pixels,
		FOS_Pts
	}type;
	float val;
};

struct FontAtlasAdditionSpec
{
	enum
	{
		FS_Normal = 1,
		FS_Italic = 2,
		FS_Bold = 4,
		FS_Underline = 8
	} fontOptions;
	char name[MAX_FONT_NAME_SIZE];
	char path[MAX_FONT_PATH_SIZE];
	struct FontSize fontSizes[MAX_NUM_FONT_SIZES];
	size_t numFontSizes;
};

void At_Init();
void At_BeginAtlas();

hSprite At_AddSprite(const char* imgPath, int topLeftXPx, int topRightYPx, int widthPx, int heightPx, const char* name);
HFont At_AddFont(const struct FontAtlasAdditionSpec* pFontSpec);
hAtlas At_EndAtlas(struct DrawContext* pDC);

struct EndAtlasOptions
{
	int initialAtlasWidth;
	int initialAtlasHeight;
	char* outDebugBitmapPath;
	bool bUseBiggestSpriteForInitialAtlasSize;
};

hAtlas At_EndAtlasEx(struct DrawContext* pDC, struct EndAtlasOptions* pOptions);
void At_DestroyAtlas(hAtlas atlas, struct DrawContext* pDC);
hSprite At_FindSprite(const char* name, hAtlas atlas);
AtlasSprite* At_GetSprite(hSprite sprite, hAtlas atlas);
hTexture At_GetAtlasTexture(hAtlas atlas);
float At_PixelsToPts(float val);
hAtlas At_LoadAtlas(xmlNode* child0, struct DrawContext* pDC);
hAtlas At_LoadAtlasEx(xmlNode* child0, struct DrawContext* pDC, struct EndAtlasOptions* pOptions);

hSprite At_TilemapIndexToSprite(hAtlas atlas, TileIndex tileIndex);

/// <param name="pSerializer">
/// binary serialzier to load or save
/// </param>
/// <param name="atlas">
/// If loading, point to an unititialized handle to output the new handle
/// If saving point to the handle of the atlas you want to save
/// </param>
void At_SerializeAtlas(struct BinarySerializer* pSerializer, hAtlas* atlas, struct DrawContext* pDC);

void At_BeginTileset(int beginI);
void At_EndTileset(int endI);

void At_SetCurrent(hAtlas atlas, DrawContext* pDC);

struct AtlasAnimation* At_FindAnim(hAtlas atlas, const char* name);

HFont Fo_FindFont(hAtlas hAtlas, const char* fontName, float sizePts);
float Fo_CharWidth(hAtlas hAtlas, HFont hFont, char c);
float Fo_CharHeight(hAtlas hAtlas, HFont hFont, char c);
float Fo_StringWidth(hAtlas hAtlas, HFont hFont, const char* stringVal);
float Fo_StringHeight(hAtlas hAtlas, HFont hFont, const char* stringVal);
AtlasSprite* Fo_GetCharSprite(hAtlas hAtlas, HFont hFont, char c);
float Fo_GetMaxYBearing(hAtlas hAtlas, HFont hFont, const char* str);

bool Fo_TryGetCharBearing(hAtlas hAtlas, HFont hFont, char c, vec2 outBearing);
bool Fo_TryGetCharAdvance(hAtlas hAtlas, HFont hFont, char c, float* outAdvance);

#endif