#ifndef TEMPDIR_H
#define TEMPDIR_H

/**
    @file FileSystemUtils.h
    @brief
    Cross platform functions for
    - copying files
    - recursively deleting directories
    - making temp directories
*/

/// @brief Get a temporary directory, you should delete this when the program closes
/// @param outBuf buffer to write to
/// @param size buffer size
int FS_GetTempDir(char* outBuf, int size);

int FS_CopyFile(char* srcPath, char* destPath);

int FS_DeleteDirRecursive(char* dirPath);

#endif
