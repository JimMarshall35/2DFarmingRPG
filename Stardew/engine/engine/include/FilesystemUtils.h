#ifndef TEMPDIR_H
#define TEMPDIR_H

#include <stdbool.h>
#include <stddef.h>

/**
    @file FileSystemUtils.h
    @brief
    Cross platform functions for
    - copying files
    - recursively deleting directories
    - making temp directories
*/

typedef char** FilesArray;

/// @brief Get a temporary directory, you should delete this when the program closes
/// @param outBuf buffer to write to
/// @param size buffer size
int FS_GetTempDir(char* outBuf, int size);

int FS_CopyFile(char* srcPath, char* destPath);

int FS_DeleteDirRecursive(char* dirPath);

bool FS_DoesFileExist(char* filePath);

FilesArray FS_GetFilesInDir(const char* dirPath, size_t* pOutFilesArraySize);

void FS_FreeFilesArray(FilesArray f, size_t size);

int FS_CreateDir(const char* dir);


#endif
