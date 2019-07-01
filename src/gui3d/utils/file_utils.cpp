#include <gui3d/base/io.h>
#include <sys/stat.h>

#if WIN32
#include <direct.h>
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#endif
#include <stdarg.h> // va_list
#include <stdio.h> // vsnprintf
#include <zconf.h>

const std::vector<std::string> explode(const std::string& s, const char& c)
{
    std::string buff{""};
    std::vector<std::string> v;

    for(auto n:s)
    {
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if(buff != "") v.push_back(buff);

    return v;
}

std::string format(const char* fmt, ...)
{
    std::string str;
    str.resize(std::max(256, (int) strlen(fmt) + 1));
    for (;;)
    {
        va_list args;
        va_start(args, fmt); // Initialize variable arguments
        const int bsize = (int) str.size();
        const int len = vsnprintf((char *) str.data(), bsize, fmt, args);
        va_end(args); // Reset variable argument list
        if (len < 0 || len >= bsize)
        {
            str.resize(std::max(bsize * 2, len + 1));
            continue;
        }
        else
        {
            str.resize(len);
            return str;
        }
    }
}

#if WIN32
bool IsDirectory(const char *pDir)
{
    char szCurPath[500];
    ZeroMemory(szCurPath, 500);
    sprintf_s(szCurPath, 500, "%s//*", pDir);
    WIN32_FIND_DATAA FindFileData;
    ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));

    HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData); /**< find first file by given path. */

    if (hFile == INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        return FALSE;
    }
    else
    {
        FindClose(hFile);
        return TRUE;
    }

}

bool DeleteDirectory(const char * DirName)
{
    //    CFileFind tempFind;
    char szCurPath[MAX_PATH];
    _snprintf(szCurPath, MAX_PATH, "%s//*.*", DirName);
    WIN32_FIND_DATAA FindFileData;
    ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
    HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
    BOOL IsFinded = TRUE;
    while (IsFinded)
    {
        IsFinded = FindNextFileA(hFile, &FindFileData);
        if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, ".."))
        {
            std::string strFileName = "";
            strFileName = strFileName + DirName + "//" + FindFileData.cFileName;
            std::string strTemp;
            strTemp = strFileName;
            if (IsDirectory(strFileName.c_str())) //
            {
                printf("dir :%s/n", strFileName.c_str());
                DeleteDirectory(strTemp.c_str());
            }
            else
            {
                DeleteFileA(strTemp.c_str());
            }
        }
    }
    FindClose(hFile);

    BOOL bRet = RemoveDirectoryA(DirName);
    if (bRet == 0) //
    {
        printf("delete dir %s failed/n", DirName);
        return FALSE;
    }
    return TRUE;

}

void removeAllFile(const std::string& dir)
{
    if (access(dir.c_str(), 0) == 0)
    {
        //DeleteDirFile(dir.c_str());
        DeleteDirectory(dir.c_str());
    }

    if (access(dir.c_str(), 0) == -1)
    {
        LOGI("%s is not existing.", dir.c_str());
        LOGI("now make it.");
        int flag = mkdir(dir.c_str());
        if (flag == 0)
            std::cout << "make successfully" << std::endl;
        else
            std::cout << "make errorly" << std::endl;
    }
}
#else
void dfs_remove_dir()
{
    DIR *cur_dir = opendir(".");
    struct dirent *ent = NULL;
    struct stat st;

    if (!cur_dir)
    {
        perror("opendir:");
        return;
    }

    while ((ent = readdir(cur_dir)) != NULL)
    {
        stat(ent->d_name, &st);

        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            int tmp = chdir(ent->d_name);
            dfs_remove_dir();
            tmp = chdir("..");
        }

        remove(ent->d_name);
    }
    closedir(cur_dir);
}

void remove_dir(const char *path_raw)
{
    char old_path[100];

    if (!path_raw)
    {
        return;
    }

    auto tmp = getcwd(old_path, 100);

    if (chdir(path_raw) == -1)
    {
        fprintf(stderr, "not a dir or access error\n");
        return;
    }

    printf("path: %s\n", path_raw);
    dfs_remove_dir();
    int n = chdir(old_path);

    /*
       unlink(old_path);
     */
}

void removeAllFile(const std::string& dir)
{
    if (access(dir.c_str(), 0) == 0)
    {
        remove_dir(dir.c_str());
    }

    if (access(dir.c_str(), 0) == -1)
    {
        LOGI("%s is not existing.", dir.c_str());
        LOGI("now make it.");
        int flag= mkdir(dir.c_str(), 0777);
        if (flag == 0)
            LOGI("make successfully");
        else
            LOGE("make errorly");
    }
}
#endif // end of #if WIN32
