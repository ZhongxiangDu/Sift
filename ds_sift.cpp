#include <iostream>
#include <vector>
#include "ds_debug.h"
#include "ds_pgm.h"
#include "ds_scalespace.h"
#include "ds_feature.h"
#include "ds_sift.h"

DS_SIFT::DS_SIFT(DS_PGM *pstPic)
    : pstScaleSpace(NULL)
    , pstFeatures(NULL)
{
    this->pstImage = pstPic;
}

DS_SIFT::~DS_SIFT(void)
{
    if (NULL != this->pstScaleSpace)
    {
        delete this->pstScaleSpace;
    }

    if (NULL != this->pstFeatures)
    {
        delete this->pstFeatures;
    }
}

/* create scale space */
int DS_SIFT::createScaleSpace(int iFirstOctave, int iLastOctave, int iFirstLevel, int iLastLevel, unsigned int uiOctaveResolution)
{
    int iRet = 0;

    this->pstScaleSpace = new DS_SCALESPACE(this->pstImage, iFirstOctave, iLastOctave, iFirstLevel, iLastLevel, uiOctaveResolution);
    if (NULL == pstScaleSpace)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create scale space.");
        return -1;
    }

    /* create scale space */
    iRet = this->pstScaleSpace->createScaleSpace();
    if (0 != iRet)
    {
        delete this->pstScaleSpace;
        this->pstScaleSpace = NULL;
    }

    return iRet;
}

/*  create features, called afer create the scale space */
int DS_SIFT::createFeatures(void)
{
    int iRet = 0;

    /* when the scale space is not created, return failed */
    if (NULL == this->pstScaleSpace)
    {
        return -1;
    }

    this->pstFeatures = new DS_FEATURE_FILTER(this->pstScaleSpace);
    if (NULL == this->pstFeatures)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create featue filter.");
        return -1;
    }

    /* create features */
    iRet = this->pstFeatures->createFeatures();
    if (0 != iRet)
    {
        delete this->pstFeatures;
        this->pstFeatures = NULL;
    }

    /* transfer feature into integer*/
    this->pstFeatures->transFeatureToInt();

    return iRet;
}

/* get all the features */
std::vector <DS_FEATURE *> *DS_SIFT::getAllFeatures(void)
{
    if (NULL == this->pstFeatures)
    {
        return NULL;
    }

    return this->pstFeatures->getAllFeatures();
}

/* get feature number */
unsigned int DS_SIFT::getFeatureNum(void)
{
    if (NULL == this->pstFeatures)
    {
        return 0;
    }
    return this->pstFeatures->getFeatureNum();
}

/* get image width */
unsigned int DS_SIFT::getWidth(void)
{
    return this->pstImage->getWidth();
}

/* get image height */
unsigned int DS_SIFT::getHeight(void)
{
    return this->pstImage->getHeight();
}

/* get image data */
float *DS_SIFT::getImageData(void)
{
    return this->pstImage->getImageData();
}
