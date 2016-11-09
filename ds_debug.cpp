#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool g_bIsDebugOpen = false;

void DS_OpenDebug()
{
    g_bIsDebugOpen = true;
    return;
}

void DS_CloseDebug()
{
    g_bIsDebugOpen = false;
    return;
}

void DS_WriteErrorInfo(char *pcFileName, int iLineNum, char *pcFormat, ...)
{
    char szTmpBuf[512];

    va_list argList;

    va_start(argList, pcFormat);
    vsnprintf(szTmpBuf, sizeof(szTmpBuf), pcFormat, argList);
    va_end(argList);

    printf("Error: %s %s:%d\n", szTmpBuf, pcFileName, iLineNum);

    return;
}

void DS_WriteDebugInfo(char *pcFormat, ...)
{
    va_list argList;
    char szTmpBuf[512];

    if (false == g_bIsDebugOpen)
    {
        return;
    }

    va_start(argList, pcFormat);
    vsnprintf(szTmpBuf, sizeof(szTmpBuf), pcFormat, argList);
    va_end(argList);

    printf("Debug: %s\n", szTmpBuf);

    return;
}
