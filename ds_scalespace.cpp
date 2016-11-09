#include <iostream>
#include <vector>
#include <math.h>
#include "ds_main.h"
#include "ds_debug.h"
#include "ds_pgm.h"
#include "ds_scalespace.h"

/**************************************************************************
    SCALE_LEVEL
***************************************************************************/
SCALE_LEVEL::SCALE_LEVEL(unsigned int uiWidth, unsigned int uiHeight, int iFirstLevel, int iLastLevel)
    : pfGaussImages(NULL)
{

    this->uiWidth     = uiWidth;
    this->uiHeight    = uiHeight;
    this->iFirstLevel = iFirstLevel;
    this->iLastLevel  = iLastLevel;
    this->uiLevelNum  = iLastLevel - iFirstLevel + 1;

    return;
}

SCALE_LEVEL::~SCALE_LEVEL()
{
    if (NULL != pfGaussImages)
    {
        delete pfGaussImages;
    }
    
    return;
}

/* set level data */
void SCALE_LEVEL::setLevelData(float *pfImgData, int iLevel)
{
    memcpy(this->pfGaussImages + (iLevel - this->iFirstLevel) * this->uiHeight * this->uiWidth,
           pfImgData,
           sizeof(float) * this->uiHeight * this->uiWidth);
    return;
}

/* get level data */
float * SCALE_LEVEL::getLevelData(int iLevel)
{
    return this->pfGaussImages + (iLevel - this->iFirstLevel) * this->uiHeight * this->uiWidth;
}

/* get data of all level */
float * SCALE_LEVEL::getData(void)
{
    return this->pfGaussImages;
}

/* create all levels */
int SCALE_LEVEL::createLevels(void)
{
    this->pfGaussImages = new float[this->uiLevelNum * uiWidth * uiHeight];
    if (NULL== this->pfGaussImages)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to alloc memery for Level.");
        return -1;
    }
    return 0;
}

/* get level height */
unsigned int SCALE_LEVEL::getHeight(void)
{
    return this->uiHeight;
}

/* get level width */
unsigned int SCALE_LEVEL::getWidth(void)
{
    return this->uiWidth;
}

/* get level number */
unsigned int SCALE_LEVEL::getLevelNum(void)
{
    return this->uiLevelNum;
}

/**************************************************************************
    DS_SCALESPACE
***************************************************************************/
DS_SCALESPACE::DS_SCALESPACE(DS_PGM *pstPic, int iFirstOctave, int iLastOctave, int iFirstLevel, int iLastLevel, unsigned int uiOctaveResolution)
    : dSigmaN(0.5)
{
    this->iFirstOctave = iFirstOctave;
    this->iLastOctave  = iLastOctave;
    this->iFirstLevel  = iFirstLevel;
    this->iLastLevel   = iLastLevel;
    this->uiOctaveNum  = iLastOctave - iFirstOctave + 1;
    this->uiOctaveResolution = uiOctaveResolution;
    this->pstPic = pstPic;
    this->dSigmaK  = pow(2.0, 1.0 / uiOctaveResolution);
    this->dSigma0  = 1.6 * this->dSigmaK;
    this->dDSigma0 = this->dSigma0 * sqrt(1.0 - 1.0 / (this->dSigmaK * this->dSigmaK));
    return;
}

DS_SCALESPACE::~DS_SCALESPACE(void)
{
    this->deleteScaleSpace();

    return;
}

/* create gaussian space and DoG */
int DS_SCALESPACE::createScaleSpace(void)
{
    int iO;
    int iRet = 0;
    unsigned int uiWidth;
    unsigned int uiHeight;
    SCALE_LEVEL *pstLevel = NULL;

    /* create gaussian space */
    for (iO = this->iFirstOctave; iO <= this->iLastOctave; iO ++)
    {
        this->getSizeByOctave(&uiWidth, &uiHeight, iO);

        /* when a level is too samll jump out of loop */
        if (4 > uiWidth || 4 > uiHeight)
        {
            this->iLastOctave = iO - 1;
            break;
        }

        pstLevel = new SCALE_LEVEL(uiWidth, uiHeight, this->iFirstLevel, this->iLastLevel);
        if (NULL == pstLevel)
        {   
            iRet = -1;
            break;
        }

        /* create the levels of an octave */
        iRet = pstLevel->createLevels();
        if (0 != iRet)
        {
            delete pstLevel;
            break;
        }
        this->vGaussOctaves.push_back(pstLevel);
    }

    if (0 != iRet)
    {
        this->deleteScaleSpace();
        return iRet;
    }

    /* fill image info into gaussian space */
    iRet = this->fillGauseSpace();
    if (0 != iRet)
    {
        this->deleteScaleSpace();
        return iRet;
    }
    
    DS_WriteDebugInfo((char *)"create gauss scale space successfully.");
    
    /* create gaussian DoG */
    for (iO = this->iFirstOctave; iO <= this->iLastOctave; iO ++)
    {
        this->getSizeByOctave(&uiWidth, &uiHeight, iO);
        pstLevel = new SCALE_LEVEL(uiWidth, uiHeight, this->iFirstLevel, this->iLastLevel - 1);
        if (NULL == pstLevel)
        {   
            iRet = -1;
            break;
        }

        /* create the levels of an octave */
        iRet = pstLevel->createLevels();
        if (0 != iRet)
        {
            delete pstLevel;
            break;
        }
        this->vGoDOctaves.push_back(pstLevel);
    }

    if (0 != iRet)
    {
        this->deleteScaleSpace();
        return iRet;
    }

    this->fillDoGSpace();

    DS_WriteDebugInfo((char *)"create DoG scale space successfully.");

    /* create the gradient and directions of DoG */
    for (iO = this->iFirstOctave; iO <= this->iLastOctave; iO ++)
    {
        this->getSizeByOctave(&uiWidth, &uiHeight, iO);
        pstLevel = new SCALE_LEVEL(uiWidth * 2, uiHeight, this->iFirstLevel, this->iLastLevel);
        if (NULL == pstLevel)
        {   
            iRet = -1;
            break;
        }

        /* create the levels of an octave */
        iRet = pstLevel->createLevels();
        if (0 != iRet)
        {
            delete pstLevel;
            break;
        }

        this->vGradGaussOctaves.push_back(pstLevel);
    }

    if (0 != iRet)
    {
        this->deleteScaleSpace();
    }

    this->fillGradGaussSpace();

    DS_WriteDebugInfo((char *)"create grad scale space successfully.");

    DS_WriteDebugInfo((char *)"create all scale space successfully Octave(%d ~ %d) Level(%d ~ %d).",
                      this->iFirstOctave, 
                      this->iLastOctave, 
                      this->iFirstLevel, 
                      this->iLastLevel);
    
    return iRet;
}

/* get the vallue of first octave */
int DS_SCALESPACE::getFirstOctave(void)
{
    return this->iFirstOctave;
}

/* get the value of last octave */
int DS_SCALESPACE::getLastOctave(void)
{
    return this->iLastOctave;
}

/* get the DoG data */
SCALE_LEVEL *DS_SCALESPACE::getDoGData(int iOctave)
{
    return this->vGoDOctaves[iOctave - this->iFirstOctave];
}

/* get gaussian data */
SCALE_LEVEL *DS_SCALESPACE::getGaussData(int iOctave)
{
    return this->vGaussOctaves[iOctave - this->iFirstOctave];
}

/* get the gradient gaussian data */
SCALE_LEVEL *DS_SCALESPACE::getGradGaussData(int iOctave)
{
    return this->vGradGaussOctaves[iOctave - this->iFirstOctave];
}

/* get the value of first level */
int DS_SCALESPACE::getFirstLevel(void)
{
    return this->iFirstLevel;
}

/* get the value of first level */
int DS_SCALESPACE::getLastLevel(void)
{
    return this->iLastLevel;
}

/* get the octave resolution */
int DS_SCALESPACE::getOctaveResolution(void)
{
    return this->uiOctaveResolution;
}

/* get the size of octave */
void DS_SCALESPACE::getSizeByOctave(unsigned int *puiWidth, unsigned int *puiHeight, int iOctave)
{
    unsigned int uiWidth;
    unsigned int uiHeight;

    uiWidth = this->pstPic->getWidth();
    uiHeight = this->pstPic->getHeight();

    if (0 > iOctave)
    {
        *puiWidth  = uiWidth << (-iOctave);
        *puiHeight = uiHeight << (-iOctave);
    }
    else
    {
        *puiWidth  = uiWidth >> iOctave;
        *puiHeight = uiHeight >> iOctave;
    }

    return;
}

/* delete all space */
void DS_SCALESPACE::deleteScaleSpace(void)
{
    SCALE_LEVEL *pstLevel = NULL;
    std::vector <SCALE_LEVEL *>::iterator iter;

    /* delete gaussian space */
    for (iter = this->vGaussOctaves.begin(); iter != this->vGaussOctaves.end(); )
    {
        pstLevel = *iter;
        delete pstLevel;
        iter = this->vGaussOctaves.erase(iter);
    }

    /* delete DoG */
    for (iter = this->vGoDOctaves.begin(); iter != this->vGoDOctaves.end(); )
    {
        pstLevel = *iter;
        delete pstLevel;
        iter = this->vGoDOctaves.erase(iter);
    }

    /* delete gradient gaussian */
    for (iter = this->vGradGaussOctaves.begin(); iter != this->vGradGaussOctaves.end(); )
    {
        pstLevel = *iter;
        delete pstLevel;
        iter = this->vGradGaussOctaves.erase(iter);
    }
    
    return;
}

/* up sample image */
void DS_SCALESPACE::upSampleImage(float *pdSrcImg, float *pfUpImage, unsigned int uiWidth, unsigned int uiHeight)
{
    unsigned int uiXpos;
    unsigned int uiYpos;
    int          iEndFlagY;              /* if has reached the bottom of original image */
    int          iEndFlagX;              /* if has reached the right side of original image */
    float        afTmp[2][2];

    for (uiYpos = 0; uiYpos < uiHeight; uiYpos ++)
    {
        iEndFlagY = uiYpos != (uiHeight - 1);
        afTmp[0][1] = pdSrcImg[0];
        afTmp[1][1] = pdSrcImg[iEndFlagY * uiWidth];
        for (uiXpos = 0; uiXpos < uiWidth; uiXpos ++)
        {
            iEndFlagX = uiXpos != (uiWidth - 1);
            
            /* select 4 neighbours if at corner select 2 */
            afTmp[0][0] = afTmp[0][1];
            afTmp[1][0] = afTmp[1][1];
            afTmp[0][1] = pdSrcImg[iEndFlagX];
            afTmp[1][1] = pdSrcImg[iEndFlagX + iEndFlagY * uiWidth];
            
            /* set value to the up sampled image */
            pfUpImage[0]                     = afTmp[0][0];
            pfUpImage[1]                     = 0.5 * (afTmp[0][0] + afTmp[0][1]);
            pfUpImage[2 * uiWidth]     = 0.5 * (afTmp[0][0] + afTmp[1][0]);
            pfUpImage[2 * uiWidth + 1] = 0.25 * (afTmp[0][0] + afTmp[0][1] + afTmp[1][0] + afTmp[1][1]);

            pfUpImage += 2;
            pdSrcImg++;
        }
        pfUpImage += 2 * uiWidth;
    }

    return;
}

/* down sample image */
void DS_SCALESPACE::downSampleImage(float *pfSrcImg, float *pfDownImage, unsigned int uiWidth, unsigned int uiHeight)
{
    unsigned int uiXpos;
    unsigned int uiYpos;
    float       *pfTmp = NULL;

    for (uiYpos = 0; uiYpos < uiHeight; uiYpos += 2)
    {
        pfTmp = pfSrcImg + uiYpos * uiWidth;
        for (uiXpos = 0; uiXpos < uiWidth; uiXpos += 2)
        {
            *pfDownImage = *pfTmp;
            pfDownImage ++;
            pfTmp += 2;
        }
    }

    return;
}

/* gaussian smooth on image */
int DS_SCALESPACE::gaussSmooth(float *pfSrcImg, float *pfDesImg, float fSigma, unsigned int uiWidth, unsigned int uiHeight)
{
    float       *pfFilter = NULL;
    unsigned int uiFilterSize;
    float       *pfTmpImg = NULL;

    /* create gaussian filter */
    pfFilter = this->createGuassFilter(&uiFilterSize, fSigma);
    if (NULL == pfFilter)
    {
        return -1;
    }

    /* create image buffer */
    pfTmpImg = new float[uiWidth * uiHeight];
    if (NULL == pfTmpImg)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed create temp image.");
        delete pfFilter;
        return -1;
    }

    /* run gaussian horizonal */
    this->convolveHeight(pfFilter, uiFilterSize, pfTmpImg, pfSrcImg, uiWidth, uiHeight);

    /* run gaussian vertical */
    this->convolveWidth(pfFilter, uiFilterSize, pfDesImg, pfTmpImg, uiWidth, uiHeight);

    delete pfFilter;
    delete pfTmpImg;

    return 0;
}

/* create gaussian filter */
float *DS_SCALESPACE::createGuassFilter(unsigned int *puiFilterSize, float fSigma )
{
    unsigned int uiLoop;
    unsigned int uiFilterSize;
    unsigned int uiSize;
    float *pfFilter = NULL;
    float  fTmp;
    float  fSum = 0.0;

    uiFilterSize = ceil(4.0 * fSigma);
    uiSize = 2 * uiFilterSize + 1;
    pfFilter = new float[uiSize];
    if (NULL == pfFilter)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed create GaussFilter.");
        return pfFilter;
    }

    *puiFilterSize = uiFilterSize;

    for (uiLoop = 0; uiLoop < uiSize; uiLoop++)
    {
        fTmp = (float)((int)uiLoop - (int)uiFilterSize) / fSigma;
        pfFilter[uiLoop] = exp(-0.5 * fTmp * fTmp);
        fSum += pfFilter[uiLoop];
    }

    for (uiLoop = 0; uiLoop < uiSize; uiLoop++)
    {
        pfFilter[uiLoop] /= fSum;
    }

    return pfFilter;
}

/* run gaussian horizonal */
void DS_SCALESPACE::convolveHeight(float        *pfFilter, 
                                   unsigned int  uiFilterSize,
                                   float        *pfDesImg, 
                                   float        *pfSrcImg,
                                   unsigned int  uiWidth, 
                                   unsigned int  uiHeight)
{
    unsigned int uiPosX;
    unsigned int uiPosY;
    unsigned int uiLoop;
    unsigned int uiSize;
    float        fTmp;

    uiSize = 2 * uiFilterSize + 1;
    for (uiPosX = 0; uiPosX < uiWidth; uiPosX ++)
    {
        for (uiPosY = 0; uiPosY < uiHeight; uiPosY ++)
        {
            fTmp = 0.0;
            for (uiLoop = 0; uiLoop < uiSize; uiLoop ++)
            {
                /* when the window reach out the image, extent the image */
                if (uiPosY + uiLoop < uiFilterSize)
                {
                    fTmp += pfFilter[uiLoop] * pfSrcImg[uiPosX];
                }
                else
                {
                    /* when the window reach out the image, extent the image */
                    if (uiPosY + uiLoop + 1 > uiFilterSize + uiHeight)
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[uiPosX + (uiHeight - 1) * uiWidth];
                    }
                    else
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[uiPosX + (uiPosY + uiLoop - uiFilterSize) * uiWidth];
                    }
                }
            }

            pfDesImg[uiPosX + uiPosY * uiWidth] = fTmp;
        }
    }

    return;
}

/* run gaussian vertical */
void DS_SCALESPACE::convolveWidth(float        *pfFilter, 
                                  unsigned int  uiFilterSize,
                                  float        *pfDesImg, 
                                  float        *pfSrcImg,
                                  unsigned int  uiWidth, 
                                  unsigned int  uiHeight)
{
    unsigned int uiPosX;
    unsigned int uiPosY;
    unsigned int uiLoop;
    unsigned int uiSize;
    float        fTmp;

    uiSize = 2 * uiFilterSize + 1;
    for (uiPosY = 0; uiPosY < uiHeight; uiPosY ++)
    {
        for (uiPosX = 0; uiPosX < uiWidth; uiPosX ++)
        {
            fTmp = 0.0;
            for (uiLoop = 0; uiLoop < uiSize; uiLoop ++)
            {
                if (uiPosX < uiFilterSize)
                {
                    /* when the window reach out the image, extent the image */
                    if (uiPosX + uiLoop < uiFilterSize)
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[uiPosY * uiWidth];
                    }
                    else
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[(uiPosX + uiLoop - uiFilterSize) + uiPosY * uiWidth];
                    }
                }
                else if (uiPosX + 1 + uiFilterSize > uiWidth)
                {
                    /* when the window reach out the image, extent the image */
                    if (uiPosX + uiLoop + 1 > uiWidth + uiFilterSize)
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[uiWidth - 1 + uiPosY * uiWidth];
                    }
                    else
                    {
                        fTmp += pfFilter[uiLoop] * pfSrcImg[(uiPosX + uiLoop - uiFilterSize) + uiPosY * uiWidth];
                    }
                }
                else
                {
                    fTmp += pfFilter[uiLoop] * pfSrcImg[(uiPosX + uiLoop - uiFilterSize) + uiPosY * uiWidth];
                }
            }

            pfDesImg[uiPosX + uiPosY * uiWidth] = fTmp;
        }
    }

    return;
}

/* fill gaussian space */
int DS_SCALESPACE::fillGauseSpace(void)
{
    unsigned int uiWidth;
    unsigned int uiHeight;
    int          iO;
    int          iRet = 0;
    int          iLevel;
    int          iBestLevel;
    float       *pfTmpBuf  = NULL;
    float       *pfTmpData = NULL;
    float       *pfTmp     = NULL;
    double       dSigma = 0.0;
    unsigned int uiLastOctaveLevel;

    /* malloc a space */
    this->getSizeByOctave(&uiWidth, &uiHeight, this->iFirstOctave);

    pfTmpBuf = new float[uiWidth * uiHeight];
    if (NULL == pfTmpBuf)
    {
        return -1;
    }

    uiLastOctaveLevel = DS_MIN(this->uiOctaveResolution, this->iLastLevel - this->iFirstLevel);
    for (iO = this->iFirstOctave; iO <= this->iLastOctave; iO ++)
    {
        this->getSizeByOctave(&uiWidth, &uiHeight, iO);

        /* when processing the  first octave */
        if (iO == this->iFirstOctave)
        {
            /* the first octave can only be 0 or -1 */
            if (-1 == this->iFirstOctave)
            {
                /* when start with -1, we need up sample */
                this->upSampleImage(this->pstPic->getImageData(), pfTmpBuf, this->pstPic->getWidth(), this->pstPic->getHeight());
            }
            else if (0 == this->iFirstOctave)
            {
                /* when start with 0, use the original image */
                memcpy(pfTmpBuf, this->pstPic->getImageData(), uiWidth * uiHeight * sizeof(float));
            }
        }
        else
        {
            iBestLevel = DS_MIN(this->iFirstLevel + this->uiOctaveResolution, this->iLastLevel);
            pfTmp = this->vGaussOctaves[iO - this->iFirstOctave - 1]->getLevelData(iBestLevel);
        
            this->downSampleImage(pfTmp, pfTmpBuf, uiWidth * 2, uiHeight * 2);
        }

        for (iLevel = this->iFirstLevel; iLevel <= this->iLastLevel; iLevel ++)
        {
            dSigma = this->calculateSigma(iO, iLevel);

            DS_WriteDebugInfo((char *)"gauss sigma is %8f (Octave:%2d Level:%2d)", dSigma, iO, iLevel);
            
            if (0.0 != dSigma)
            {
                pfTmp = this->vGaussOctaves[iO - this->iFirstOctave]->getLevelData(iLevel);
                
                if (iLevel == this->iFirstLevel)
                {
                    /* hen the current level is the first, use the image of the last sampled */
                    iRet = this->gaussSmooth(pfTmpBuf, pfTmp, dSigma, uiWidth, uiHeight);
                }
                else
                {
                    /* when the current level is not the first, use the image of last level */
                    pfTmpData = this->vGaussOctaves[iO - this->iFirstOctave]->getLevelData(iLevel - 1);
                    iRet = this->gaussSmooth(pfTmpData, pfTmp, dSigma, uiWidth, uiHeight);
                }
                
                if (0 != iRet)
                {
                    break;
                }
            }
            else
            {
                this->vGaussOctaves[iO - this->iFirstOctave]->setLevelData(pfTmpBuf, iLevel);
            }
        }

        if (0 != iRet)
        {
            break;
        }
    }

    delete pfTmpBuf;
    
    return iRet;
}

/* calculate the sigma of current octave and level */
double DS_SCALESPACE::calculateSigma(int iOctave, int iLevel)
{
    double dSigma = 0.0;
    double dSigmaCur = 0.0;
    double dSigmaPre = 0.0;
    int    iLevelTmp;

    dSigmaCur = this->dSigma0 * pow(this->dSigmaK, this->iFirstLevel);

    /* when proc the first octave */
    if (iOctave == this->iFirstOctave)
    {         
        /* when proc the first level */
        if (iLevel == this->iFirstLevel)
        {
            dSigmaPre = this->dSigmaN * pow(2.0, -this->iFirstOctave);
            if (dSigmaCur > dSigmaPre)
            {
                dSigma = sqrt(dSigmaCur * dSigmaCur - dSigmaPre * dSigmaPre);
            }
            else
            {
                dSigma = 0.0;
            }
        }
        else
        {
            dSigma = this->dDSigma0 * pow(this->dSigmaK, iLevel);
        }
    }
    else
    {
        /* when proc the first level */
        if (iLevel == this->iFirstLevel)
        {
            iLevelTmp = DS_MIN(this->iFirstLevel + this->uiOctaveResolution, this->iLastLevel);
            dSigmaPre = this->dSigma0 * pow(this->dSigmaK, iLevelTmp - (int)this->uiOctaveResolution);
        
            if (dSigmaCur > dSigmaPre)
            {
                dSigma = sqrt(dSigmaCur * dSigmaCur - dSigmaPre * dSigmaPre);
            }
            else
            {
                dSigma = 0.0;
            }
        }
        else
        {
            dSigma = this->dDSigma0 * pow(this->dSigmaK, iLevel);
        }
    }

    return dSigma;
}

/* fill DoG space */
void DS_SCALESPACE::fillDoGSpace(void)
{
    int iOctave;
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiPosX;
    unsigned int uiPosY;
    unsigned int uiPos;
    int iLevel;
    SCALE_LEVEL *pstGauss    = NULL;
    SCALE_LEVEL *pstDoG      = NULL;
    float       *pfGaussCur  = NULL;
    float       *pfGaussNext = NULL;
    float       *pfDoG       = NULL;

    for (iOctave = this->iFirstOctave; iOctave <= this->iLastOctave; iOctave ++)
    {
        pstGauss = this->vGaussOctaves[iOctave - this->iFirstOctave];
        pstDoG   = this->vGoDOctaves[iOctave - this->iFirstOctave];

        this->getSizeByOctave(&uiWidth, &uiHeight, iOctave);
        for (iLevel = this->iFirstLevel; iLevel <= this->iLastLevel - 1; iLevel ++)
        {
            pfGaussCur  = pstGauss->getLevelData(iLevel);
            pfGaussNext = pstGauss->getLevelData(iLevel + 1);
            pfDoG       = pstDoG->getLevelData(iLevel);
            for (uiPosX = 0; uiPosX < uiWidth; uiPosX ++)
            {
                for (uiPosY = 0; uiPosY < uiHeight; uiPosY ++)
                {
                    uiPos = uiPosX + uiPosY * uiWidth;
                    pfDoG[uiPos] = pfGaussNext[uiPos] - pfGaussCur[uiPos];
                }
            }
        }
    }

    return;
}

/* calculate the gradient and direction */
void DS_SCALESPACE::calcCurGradient(float *pfGauss, float *pfGradGauss, unsigned int uiWidth, unsigned int uiHeight)
{
    unsigned int uiLoop;
    float      *pfTmp = NULL;
    float       fDx;
    float       fDy;

#define SAVE_RESULT                                 \
    *pfGradGauss = sqrt(fDx * fDx + fDy * fDy);     \
     pfGradGauss ++;                                \
    *pfGradGauss = atan2(fDy, fDx) + DS_PI;         \
     pfGradGauss ++;                                \
     pfGauss ++;   

    /* calculate the first pixel of first row */
    fDx = pfGauss[1]       - pfGauss[0];
    fDy = pfGauss[uiWidth] - pfGauss[0];
    SAVE_RESULT;

    /* calculate the pixel of first row */
    pfTmp = pfGauss + uiWidth - 2;
    while (pfGauss < pfTmp)
    {
        fDx = 0.5 * (pfGauss[1] - pfGauss[-1]);
        fDy = pfGauss[uiWidth] - pfGauss[0];
        SAVE_RESULT;
    }

    /* calculate the last pixel of first row */
    fDx = pfGauss[0]       - pfGauss[-1];
    fDy = pfGauss[uiWidth] - pfGauss[0];
    SAVE_RESULT;

    /* calculate the pixels of middle row */
    for (uiLoop = 1; uiLoop < uiHeight - 1; uiLoop ++)
    {
        /* calculate the first pixel of middle row */
        fDx = pfGauss[1] - pfGauss[0];
        fDy = 0.5 * (pfGauss[uiWidth] - pfGauss[0]);
        SAVE_RESULT;

        /* calculate the pixel of middle row*/
        pfTmp = pfGauss + uiWidth - 2;
        while (pfGauss < pfTmp)
        {
            fDx = 0.5 * (pfGauss[1]       - pfGauss[-1]);
            fDy = 0.5 * (pfGauss[uiWidth] - pfGauss[-(int)uiWidth]);
            SAVE_RESULT;
        }

        /* calculate the last pixel of middle row */
        fDx = pfGauss[0] - pfGauss[-1];
        fDy = 0.5 * (pfGauss[uiWidth] - pfGauss[-(int)uiWidth]);
        SAVE_RESULT;
    }

    /* calculate the first pixel of last row */
    fDx = pfGauss[1] - pfGauss[0];
    fDy = pfGauss[0] - pfGauss[-(int)uiWidth];
    SAVE_RESULT;

    /* calculate the pixel of last row */
    pfTmp = pfGauss + uiWidth - 2;
    while (pfGauss < pfTmp)
    {
        fDx = 0.5 * (pfGauss[1] - pfGauss[-1]);
        fDy = pfGauss[0] - pfGauss[-(int)uiWidth];
        SAVE_RESULT;
    }

    /* calculate the last pixel of last row */
    fDx = pfGauss[0] - pfGauss[-1];
    fDy = pfGauss[0] - pfGauss[-(int)uiWidth];
    SAVE_RESULT;

#undef SAVE_RESULT
    return;
}

/* fill gradient gaussian space*/
void DS_SCALESPACE::fillGradGaussSpace(void)
{
    int iOctave;
    unsigned int uiWidth;
    unsigned int uiHeight;
    int iLevel;
    SCALE_LEVEL *pstGauss     = NULL;
    SCALE_LEVEL *pstGradGauss = NULL;
    float       *pfGauss      = NULL;
    float       *pfGradGauss  = NULL;

    for (iOctave = this->iFirstOctave; iOctave <= this->iLastOctave; iOctave ++)
    {
        pstGauss     = this->vGaussOctaves[iOctave - this->iFirstOctave];
        pstGradGauss = this->vGradGaussOctaves[iOctave - this->iFirstOctave];

        this->getSizeByOctave(&uiWidth, &uiHeight, iOctave);
        for (iLevel = this->iFirstLevel; iLevel <= this->iLastLevel; iLevel ++)
        {
            pfGauss     = pstGauss->getLevelData(iLevel);
            pfGradGauss = pstGradGauss->getLevelData(iLevel);

            this->calcCurGradient(pfGauss, pfGradGauss, uiWidth, uiHeight);
        }
    }

    return;
}

