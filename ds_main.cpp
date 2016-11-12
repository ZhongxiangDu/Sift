#include <iostream>
#include <vector>
#include <math.h>
#include "ds_debug.h"
#include "ds_main.h"
#include "ds_pgm.h"
#include "ds_scalespace.h"
#include "ds_feature.h"
#include "ds_sift.h"
#include "ds_match.h"

DS_SIFT *createSift(char *pcFilePath, DS_PGM **ppstPic)
{
    DS_PGM  *pstImage = NULL;
    DS_SIFT *pstSift = NULL;
    int      iRet = 0;

    /* create image instance */
    pstImage = new DS_PGM(pcFilePath);
    if (NULL == pstImage)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create picture instance.");
        return NULL;
    }
    
    /* create image information */
    iRet = pstImage->createImage();
    if (0 != iRet)
    {
        delete pstImage;
        return NULL;
    }

    /*  create sift instance */
    pstSift = new DS_SIFT(pstImage);
    if (NULL == pstSift)
    {
        delete pstImage;
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create sift instance.");
        return NULL;
    }

    /* create scale space */
    iRet = pstSift->createScaleSpace(-1, 5, -1, 4, 3);
    if (0 != iRet)
    {
        delete pstImage;
        delete pstSift;
        return NULL;
    }

    /* create features */
    iRet = pstSift->createFeatures();
    if (0 != iRet)
    {
        delete pstImage;
        delete pstSift;
        return NULL;
    }

    *ppstPic = pstImage;
    return pstSift;
}

int main(int argc, char* argv[])
{
    DS_PGM  *pstImageFirst  = NULL;
    DS_SIFT *pstSiftFirst   = NULL;
    DS_PGM  *pstImageSecond = NULL;
    DS_SIFT *pstSiftSecond  = NULL;
    DS_MATCH *pstMatch = NULL;
    int      iRet = 0;
    
    /* open debug mode */
    DS_OpenDebug();

    pstSiftFirst = createSift(argv[1], &pstImageFirst);
    if (NULL == pstSiftFirst)
    {
        return -1;
    }

    pstSiftSecond = createSift(argv[2], &pstImageSecond);
    if (NULL == pstSiftSecond)
    {
        delete pstSiftFirst;
        delete pstImageFirst;
        return -1;
    }

    pstMatch = new DS_MATCH(pstSiftFirst, pstSiftSecond);
    if (NULL == pstMatch)
    {
        delete pstSiftFirst;
        delete pstSiftSecond;
        delete pstImageFirst;
        delete pstImageSecond;
        return -1;
    }

    iRet = pstMatch->matchPictures();
    if (0 != iRet)
    {
        delete pstSiftFirst;
        delete pstSiftSecond;
        delete pstImageFirst;
        delete pstImageSecond;
        delete pstMatch;
        return -1;
    }

    /* remove the bad matches */
    pstMatch->ransacFilter();

    iRet = pstMatch->drawResult(argv[3]);


    delete pstSiftFirst;
    delete pstSiftSecond;
    delete pstImageFirst;
    delete pstImageSecond;
    delete pstMatch;

    return iRet;
}
