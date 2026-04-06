#include "FilesystemUtils.h"


#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <string.h>
#include "Log.h"


int FS_GetTempDir(char* outBuf, int size)
{
    #ifdef WIN32
    char base[MAX_PATH];
    char name[MAX_PATH];

    DWORD len = GetTempPathA(sizeof(base), base);
    if (len == 0 || len > sizeof(base))
        return -1;

    UINT u = GetTempFileNameA(base, "app", 0, name);
    if (u == 0)
        return -1;

    DeleteFileA(name);

    if (_mkdir(name) != 0)
        return -1;

    strncpy(outBuf, name, size);
    return 0;
    #else
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";

    char templ[512];
    snprintf(templ, sizeof(templ), "%s/StardewEngineXXXXXX", tmp);

    char *res = mkdtemp(templ);
    if (!res)
        return -1;

    strncpy(outBuf, res, size);
    return 0;
    #endif
}

int FS_DeleteDirRecursive(char* path)
{
    #ifdef _WIN32

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);

    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(pattern, &data);

    if (h == INVALID_HANDLE_VALUE)
        return -1;

    do 
    {
        if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
            continue;

        char buf[512];
        snprintf(buf, sizeof(buf), "%s\\%s", path, data.cFileName);

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            FS_DeleteDirRecursive(buf);
            RemoveDirectoryA(buf);
        }
        else
        {
            DeleteFileA(buf);
        }

    } while (FindNextFileA(h, &data));

    FindClose(h);
    return RemoveDirectoryA(path) ? 0 : -1;

#else

    DIR* dir = opendir(path);
    if (!dir) return -1;

    struct dirent* entry;

    while ((entry = readdir(dir)))
    {

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char buf[512];
        snprintf(buf, sizeof(buf), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(buf, &st) == -1)
            continue;

        if (S_ISDIR(st.st_mode)) 
        {
            FS_DeleteDirRecursive(buf);
            rmdir(buf);
        }
        else
        {
            unlink(buf);
        }
    }

    closedir(dir);
    return rmdir(path);
#endif
}

int FS_CopyFile(char* src, char* dst)
{
    FILE* psrc = fopen(src, "rb");
    if (!psrc) 
    {
        Log_Error("fopen src");
        return -1;
    }

    // Determine file size using fseek + ftell
    if (fseek(psrc, 0, SEEK_END) != 0) 
    {
        Log_Error("fseek");
        fclose(psrc);
        return -1;
    }

    long filesize = ftell(psrc);
    if (filesize < 0)
    {
        Log_Error("ftell");
        fclose(psrc);
        return -1;
    }

    if (fseek(psrc, 0, SEEK_SET) != 0) 
    {
        Log_Error("fseek rewind");
        fclose(psrc);
        return -1;
    }

    // Allocate buffer
    char *buffer = (char *)malloc(filesize);
    if (!buffer)
    {
        Log_Error("malloc");
        fclose(psrc);
        return -1;
    }

    // Read file into buffer
    size_t bytes_read = fread(buffer, 1, filesize, psrc);
    if (bytes_read != filesize)
    {
        Log_Error("fread");
        free(buffer);
        fclose(psrc);
        return -1;
    }

    fclose(psrc);

    // Write to destination
    FILE* pdst = fopen(dst, "wb");
    if (!dst)
    {
        Log_Error("fopen dst");
        free(buffer);
        return -1;
    }

    size_t bytes_written = fwrite(buffer, 1, filesize, pdst);
    if (bytes_written != filesize)
    {
        Log_Error("fwrite");
        free(buffer);
        fclose(pdst);
        return -1;
    }

    free(buffer);
    fclose(pdst);

    return 0;  // success
}


bool FS_DoesFileExist(char* filePath)
{
#ifdef _WIN32
    return _access(filePath, 0) == 0;
#else
    return access(filePath, F_OK) == 0;
#endif
}