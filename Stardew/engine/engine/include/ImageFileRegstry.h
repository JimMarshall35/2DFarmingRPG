#ifndef IMAGE_FILE_REGISTRY_H
#define IMAGE_FILE_REGISTRY_H
#include "IntTypes.h"
#include <stdbool.h>
#include "DynArray.h"
#include "HandleDefs.h"

#define CHANNELS_PER_PIXEL 4

struct ImageFile
{
	char* path;
	bool bLoaded;
	u8* pData;
	int width, height;
};

struct ImageLoadError
{
	char* message;
};


HImage IR_RegisterImagePath(const char* path);

HImage IR_LookupHandleByPath(const char* path);

bool IR_IsImageLoaded(HImage hImage);

u8* IR_GetImageData(HImage img);

const struct ImageFile* IR_GetImageFile(HImage img);

bool IR_LoadImageSync(HImage hImage, VECTOR(struct ImageLoadError) outErrors);

void IR_InitImageRegistry(const char* jsonPath);

void IR_DestroyImageRegistry();

#endif
