#include "ImageFileRegstry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "FileHelpers.h"
#include "main.h"
#include "Log.h"
#include "cwalk.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static VECTOR(struct ImageFile) gImageFiles;

HImage IR_RegisterImagePath(const char* path)
{
    
    HImage i = NULL_HIMAGE;
    struct ImageFile imagef;
    memset(&imagef, 0, sizeof(struct ImageFile));
    int bufsize = strlen(path) + 2 + strlen(gCmdArgs.assetsDir);
    imagef.path = malloc(bufsize);
    if (!imagef.path)
    {
        Log_Error("IR_RegisterImagePath malloc failed");
        return i;
    }
    cwk_path_join(gCmdArgs.assetsDir, path, imagef.path, bufsize); 
    gImageFiles = VectorPush(gImageFiles, &imagef);
    i = VectorSize(gImageFiles) - 1;
    return i;
}

HImage IR_LookupHandleByPath(const char* path)
{
    for (int i = 0; i < VectorSize(gImageFiles); i++)
    {
        if (strcmp(gImageFiles[i].path, path) == 0)
        {
            return i;
        }
    }
    return NULL_HIMAGE;
}

bool IR_IsImageLoaded(HImage hImage)
{
    if (hImage >= VectorSize(gImageFiles))
    {
        Log_Error("IR_IsImageLoaded hImage %i out of range", hImage);
        return false;
    }
    return gImageFiles[hImage].bLoaded;
}

u8* IR_GetImageData(HImage img)
{
    if (img >= VectorSize(gImageFiles))
    {
        Log_Error("IR_GetImageData hImage %i out of range", img);
        return NULL;
    }
    if (!gImageFiles[img].bLoaded)
    {
        VECTOR(struct ImageLoadError) errors = NEW_VECTOR(struct ImageLoadError);
        IR_LoadImageSync(img, errors);
        if (VectorSize(errors))
        {
            for (int i = 0; i < VectorSize(errors); i++)
            {
                Log_Error("%s", errors->message);
                free(errors->message);
            }
            DestoryVector(errors);
            return NULL;
        }
        DestoryVector(errors);
    }
    return gImageFiles[img].pData;
}

const struct ImageFile* IR_GetImageFile(HImage hImage)
{
    if (hImage >= VectorSize(gImageFiles))
    {
        Log_Error("IR_LoadImageSync hImage %i out of range", hImage);
        return false;
    }

    return &gImageFiles[hImage];
}

bool IR_LoadImageSync(HImage hImage, VECTOR(struct ImageLoadError) outErrors)
{
    if (hImage >= VectorSize(gImageFiles))
    {
        Log_Error("IR_LoadImageSync hImage %i out of range", hImage);
        return false;
    }
    struct ImageFile* pIF = &gImageFiles[hImage];
    if (pIF->bLoaded)
    {
        Log_Error("Image %i already loaded!", hImage);
        return false;
    }
    //stbi_set_flip_vertically_on_load(true);
    int x, y, n;
    u8* data = stbi_load(pIF->path, &x, &y, &n, 4);
    if (!data)
    {
        Log_Error("IR_LoadImageSync stbi_load failed");
        return false;
    }
    pIF->pData = data;
    pIF->height = y;
    pIF->width = x;

    return true;
}

void IR_InitImageRegistry(const char* jsonPath)
{
    Log_Verbose("IR_InitImageRegistry path: %s", jsonPath);
    gImageFiles = NEW_VECTOR(struct ImageFile);
    int size = 0;
    char* data = NULL;
    if (jsonPath == NULL)
    {
        data = LoadFile("./WfAssets/ImageFiles.json", &size);
    }
    else
    {
        data = LoadFile(jsonPath, &size);
    }
    
    if (!data)
    {
        Log_Error("IR_InitImageRegistry can't load config file");
        return;
    }
    cJSON* pJSON = cJSON_ParseWithLength(data, size);
    if (!pJSON)
    {
        free(data);
        Log_Error("IR_InitImageRegistry can't parse config file");
        return;
    }

    cJSON* pImgReg = cJSON_GetObjectItem(pJSON, "ImageFileRegistry");
    for (int i = 0; i < cJSON_GetArraySize(pImgReg); i++)
    {
        cJSON* pItem = cJSON_GetArrayItem(pImgReg, i);
        const char* path = pItem->valuestring;
        IR_RegisterImagePath(path);
    }

    //cJSON_Delete(pJSON);
    free(data);

}

void IR_DestroyImageRegistry()
{
    for (int i = 0; i < VectorSize(gImageFiles); i++)
    {
        if (gImageFiles[i].bLoaded)
        {
            stbi_image_free(gImageFiles->pData);
        }
        free(gImageFiles[i].path);
    }
    DestoryVector(gImageFiles);
}
