#ifndef _DS_DEBUG_H_
#define _DS_DEBUG_H_

/* open debug mode */
void DS_OpenDebug();

/* close debug mode */
void DS_CloseDebug();

/* write error info */
void DS_WriteErrorInfo(char *pcFileName, int iLineNum, char *pcFormat, ...);

/* write debug info */
void DS_WriteDebugInfo(char *pcFormat, ...);

#endif
