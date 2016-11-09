#include <iostream>
#include <vector>
#include <math.h>
#include "ds_main.h"
#include "ds_debug.h"
#include "ds_pgm.h"
#include "ds_scalespace.h"
#include "ds_feature.h"

/**************************************************************************
    DS_FEATURE
***************************************************************************/
DS_FEATURE::DS_FEATURE(float fPosX, float fPosY, float fPosZ, int iOctave, unsigned int uiWidth, unsigned int uiHeight)
    : uiOrientationNum(0)
{
    this->fPosX = fPosX;
    this->fPosY = fPosY;
    this->fPosZ = fPosZ;
    this->uiWidth = uiWidth;
    this->uiHeight = uiHeight;
    this->iOctave = iOctave;
}

DS_FEATURE::~DS_FEATURE(void)
{
}

/* get the octave NO. of current feature */
int DS_FEATURE::getOctave(void)
{
    return this->iOctave;
}

/* get width of current feature */
unsigned int DS_FEATURE::getWidth(void)
{
    return this->uiWidth;
}

/* get height of current feature */
unsigned int DS_FEATURE::getHeight(void)
{
    return this->uiHeight;
}

/* get position of current feature in scale space */
void DS_FEATURE::getFeaturePos(float *pfX, float *pfY, float *pfZ)
{
    if (NULL != pfX)
    {
        *pfX = this->fPosX;
    }

    if (NULL != pfY)
    {
        *pfY = this->fPosY;
    }

    if (NULL != pfZ)
    {
        *pfZ = this->fPosZ;
    }

    return;
}

/* get position of current feature in real space */
void DS_FEATURE::getRealFeaturePos(unsigned int *puiX, unsigned int *puiY)
{
    float fStep;
    fStep = pow(2.0, this->iOctave);

    if (NULL != puiX)
    {
        *puiX = (unsigned int)(this->fPosX * fStep);
    }

    if (NULL != puiY)
    {
        *puiY = (unsigned int)(this->fPosY * fStep);
    }

    return;
}

/* set position of current feature in real space */
void DS_FEATURE::setFeaturePos(float fPosX, float fPosY)
{
    this->fPosX = fPosX;
    this->fPosY = fPosY;
    return;
}

/* set Sigma */
void DS_FEATURE::setSigma(float fSigma)
{
    this->fSigma = fSigma;
    return;
}

/* get Sigma */
float DS_FEATURE::getSigma(void)
{
    return this->fSigma;
}

/* get number of main orientation */
unsigned int DS_FEATURE::getOrientationNum(void)
{
    return this->uiOrientationNum;
}

/* add a main orientation */
void DS_FEATURE::addOrientation(float fOrientation)
{
    /* support at most 4 main orientations */
    if (4 == this->uiOrientationNum)
    {
        return;
    }

    this->afOrientations[this->uiOrientationNum] = fOrientation;
    this->uiOrientationNum++;
    return;
}

/* get all main orientations */
void DS_FEATURE::getOrientation(float *pfOrientations)
{
    memcpy(pfOrientations, this->afOrientations, sizeof(this->afOrientations));

    return;
}

/* get the 128-dim vector of current feature */
float *DS_FEATURE::getFeature(unsigned int uiOrientationNum)
{
    /* support at most 4 main orientations */
    if (uiOrientationNum > this->uiOrientationNum)
    {
        return NULL;
    }

    return this->afFeature[uiOrientationNum];
}

/* add a 128-dim vector to current feature */
void DS_FEATURE::setFeature(float *pfFeature, unsigned int uiOrientationNum)
{
    /* support at most 4 main orientations */
    if (uiOrientationNum > this->uiOrientationNum)
    {
        return;
    }

    memcpy(this->afFeature[uiOrientationNum], pfFeature, 128 * sizeof(float));
    return ;
}

/* normalize feature*/
void DS_FEATURE::normalizeFeature(void)
{
    unsigned int uiLoopI;
    unsigned int uiLoopJ;
    float        fSum = 0.0;

    for (uiLoopI = 0; uiLoopI < this->uiOrientationNum; uiLoopI ++)
    {
        /* normalize */
        fSum = 0.0;
        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            fSum += this->afFeature[uiLoopI][uiLoopJ] * this->afFeature[uiLoopI][uiLoopJ];
        }

        fSum = sqrt(fSum);

        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            this->afFeature[uiLoopI][uiLoopJ] /= fSum;
        }

        /* remove the component larger than 0.2 */
        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            if (this->afFeature[uiLoopI][uiLoopJ] > 0.2)
            {
                this->afFeature[uiLoopI][uiLoopJ] = 0.2;
            }
        }

        /* normalize  */
        fSum = 0.0;
        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            fSum += this->afFeature[uiLoopI][uiLoopJ] * this->afFeature[uiLoopI][uiLoopJ];
        }

        fSum = sqrt(fSum);

        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            this->afFeature[uiLoopI][uiLoopJ] /= fSum;
        }
    }

    return ;
}

/* transfer into 0~255 integer */
void DS_FEATURE::transFeatureToInt(void)
{
    unsigned int uiLoopI;
    unsigned int uiLoopJ;
    int          iTmp;

    for (uiLoopI = 0; uiLoopI < this->uiOrientationNum; uiLoopI ++)
    {
        for (uiLoopJ = 0; uiLoopJ < 128; uiLoopJ ++)
        {
            iTmp = (int)(512.0 * this->afFeature[uiLoopI][uiLoopJ]);
            this->afFeature[uiLoopI][uiLoopJ] = iTmp;
        }
    }
}

/**************************************************************************
    DS_FEATURE_FILTER
***************************************************************************/

/* compare with its 26 neighbour */
#define COMPARE_NEIGHBOURS(value, CMP, x, y ,z)     \
(                                                   \
    *value CMP *(value + x) &&                      \
    *value CMP *(value - x) &&                      \
    *value CMP *(value + y) &&                      \
    *value CMP *(value - y) &&                      \
    *value CMP *(value + z) &&                      \
    *value CMP *(value - z) &&                      \
                                                    \
    *value CMP *(value + x + y) &&                  \
    *value CMP *(value + x - y) &&                  \
    *value CMP *(value - x + y) &&                  \
    *value CMP *(value - x - y) &&                  \
                                                    \
    *value CMP *(value + x + z) &&                  \
    *value CMP *(value + x - z) &&                  \
    *value CMP *(value - x + z) &&                  \
    *value CMP *(value - x - z) &&                  \
                                                    \
    *value CMP *(value + y + z) &&                  \
    *value CMP *(value + y - z) &&                  \
    *value CMP *(value - y + z) &&                  \
    *value CMP *(value - y - z) &&                  \
                                                    \
    *value CMP *(value + x + y + z) &&              \
    *value CMP *(value + x + y - z) &&              \
    *value CMP *(value + x - y + z) &&              \
    *value CMP *(value + x - y - z) &&              \
    *value CMP *(value - x + y + z) &&              \
    *value CMP *(value - x + y - z) &&              \
    *value CMP *(value - x - y + z) &&              \
    *value CMP *(value - x - y - z)                 \
)

DS_FEATURE_FILTER::DS_FEATURE_FILTER(DS_SCALESPACE *pstScaleSpace)
    : fPeakThreshold(0.01)
    , fEdgeThreshold(10.0)
    , uiFeatureNum(0)
{
    this->pstScaleSpace = pstScaleSpace;
}

DS_FEATURE_FILTER::~DS_FEATURE_FILTER(void)
{
    this->deleteFeatures();
}

/* create all features */
int DS_FEATURE_FILTER::createFeatures(void)
{
    int iRet = 0;
    int iFirstOctave;
    unsigned int uiWidth;
    unsigned int uiHeight;

    /* get the size of buttom level and malloc buffer */
    iFirstOctave = this->pstScaleSpace->getFirstOctave();
    uiWidth = this->pstScaleSpace->getGaussData(iFirstOctave)->getWidth();
    uiHeight = this->pstScaleSpace->getGaussData(iFirstOctave)->getHeight();

    /* get peak point */
    iRet = this->getExtremeFeature();
    if (0 != iRet)
    {
        return iRet;
    }

    DS_WriteDebugInfo((char *)"%d extream points are found!", this->vFeatures.size());

    /* as peak point is in the discrete space calculate it position in the real space */
    this->checkExtreamFeature();
    
    DS_WriteDebugInfo((char *)"%d extream points are left after Taylor fitting!", this->vFeatures.size());

    /* calculate all features */
    this->calcKeypoints();

    DS_WriteDebugInfo((char *)"%d feature vectors are found!", this->uiFeatureNum);

    return iRet;
}

/* transfer into 0~255 integer */
void DS_FEATURE_FILTER::transFeatureToInt(void)
{
    DS_FEATURE *pstFeature = NULL;
    std::vector <DS_FEATURE *>::iterator iter;


    for (iter = this->vFeatures.begin(); iter != this->vFeatures.end(); )
    {
        pstFeature = *iter;
        pstFeature->transFeatureToInt();
        iter ++;
    }
}

/* get all features */
std::vector <DS_FEATURE *> *DS_FEATURE_FILTER::getAllFeatures(void)
{
    return &this->vFeatures;
}

/* get feature number */
unsigned int DS_FEATURE_FILTER::getFeatureNum(void)
{
    return this->uiFeatureNum;
}

/* get peak point */
int DS_FEATURE_FILTER::getExtremeFeature(void)
{
    int iRet = 0;
    int iOctave;
    int iFirstOctave;
    int iLastOctave;
    unsigned int   uiWidth;
    unsigned int   uiHeight;
    unsigned int   uiLevelNum;
    unsigned int   uiX;
    unsigned int   uiY;
    unsigned int   uiZ;
    unsigned int   uiTmpX;
    unsigned int   uiTmpY;
    unsigned int   uiTmpZ;
    SCALE_LEVEL *pstScaleLevel = NULL;
    float *pfScaleData = NULL;
    float *pfCurData = NULL;
    DS_FEATURE *pstFeature = NULL;

    iFirstOctave = pstScaleSpace->getFirstOctave();
    iLastOctave  = pstScaleSpace->getLastOctave();

    for (iOctave = iFirstOctave; iOctave < iLastOctave - 1; iOctave ++)
    {
        /* saerch the peakpoint in the DoG */
        pstScaleLevel = pstScaleSpace->getDoGData(iOctave);

        pfScaleData = pstScaleLevel->getData();
        uiWidth     = pstScaleLevel->getWidth();
        uiHeight    = pstScaleLevel->getHeight();
        uiLevelNum  = pstScaleLevel->getLevelNum();

        uiTmpX = 1;
        uiTmpY = uiWidth;
        uiTmpZ = uiWidth * uiHeight;

        pfCurData = pfScaleData + uiTmpX + uiTmpY + uiTmpZ;

        for (uiZ = 1; uiZ < uiLevelNum - 1; uiZ ++)
        {
            for (uiY = 1; uiY < uiHeight - 1; uiY ++)
            {
                for (uiX = 1; uiX < uiWidth - 1; uiX ++)
                {
                    if ((COMPARE_NEIGHBOURS(pfCurData, >, uiTmpX, uiTmpY, uiTmpZ) && *pfCurData >   0.8 * this->fPeakThreshold) ||
                        (COMPARE_NEIGHBOURS(pfCurData, <, uiTmpX, uiTmpY, uiTmpZ) && *pfCurData <  -0.8 * this->fPeakThreshold))
                    {
                        pstFeature = new DS_FEATURE(uiX, uiY, uiZ, iOctave, uiWidth, uiHeight);
                        if (NULL == pstFeature)
                        {
                            iRet = -1;
                            break;
                        }
                        this->vFeatures.push_back(pstFeature);
                    }

                    pfCurData += uiTmpX;
                }

                if (0 != iRet)
                {
                    break;
                }

                /* if plus 1 there will be repeat calculations */
                pfCurData += 2 * uiTmpX;
            }

            if (0 != iRet)
            {
                break;
            }

            pfCurData += 2 * uiTmpY;
        }

        if (0 != iRet)
        {
            break;
        }
    }

    if (0 != iRet)
    {
        this->deleteFeatures();
    }

    return iRet;
}

#define GET_PIXEL(dx, dy, dz) \
    (*(pfTmpData + (dx) * (int)uiStrideX + (dy) * (int)uiStrideY + (dz) * (int)uiStrideZ))

/* cheak the peak points */
void DS_FEATURE_FILTER::checkExtreamFeature(void)
{
    unsigned int uiLoop;
    unsigned int uiStrideX = 1;
    unsigned int uiStrideY;
    unsigned int uiStrideZ;
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiDepth;
    unsigned int uiOctaveResolution;
    int          iX;
    int          iY;
    int          iZ;
    float        fX;
    float        fY;
    float        fZ;
    int          iOffsetX;
    int          iOffsetY;
    int          iOctave;
    int          iFirstLevel;
    float        fDx;
    float        fDy;
    float        fDz;
    float        fDxx;
    float        fDyy;
    float        fDzz;
    float        fDxy;
    float        fDxz;
    float        fDyz;
    float        afDerivativeMatrix[3][4];
    float        afResult[3];
    float       *pfDoGData = NULL;
    float       *pfTmpData = NULL;
    float        fPeakScore;
    float        fGamer;
    float        fSigma;
    SCALE_LEVEL *pstDoGOctave = NULL;
    DS_FEATURE *pstFeature = NULL;
    std::vector <DS_FEATURE *>::iterator iter;
    
    for (iter = this->vFeatures.begin(); iter != this->vFeatures.end(); )
    {
        pstFeature = *iter;
        uiWidth    = pstFeature->getWidth();
        uiHeight   = pstFeature->getHeight();

        /* init the stride */
        uiStrideY = uiWidth;
        uiStrideZ = uiWidth * uiHeight;

        /* get the info of DoG */
        iOctave = pstFeature->getOctave();
        pstDoGOctave = this->pstScaleSpace->getDoGData(iOctave);
        pfDoGData     = pstDoGOctave->getData();
        iFirstLevel  = this->pstScaleSpace->getFirstLevel();
        
        /* As DoG is one level less, so minus 1 here */
        uiDepth      = this->pstScaleSpace->getLastLevel() - iFirstLevel - 1;
        uiOctaveResolution = this->pstScaleSpace->getOctaveResolution();
        iOffsetX = 0;
        iOffsetY = 0;

        /* get the feature position in the scale space */
        pstFeature->getFeaturePos(&fX, &fY, &fZ);
        iX = fX;
        iY = fY;
        iZ = fZ;

        for (uiLoop = 0; uiLoop < 5; uiLoop ++)
        {
            iX += iOffsetX;
            iY += iOffsetY;

            pfTmpData = pfDoGData + iX * uiStrideX + iY * uiStrideY + iZ * uiStrideZ;

            /* calculate the gradient */
            fDx = 0.5 * (GET_PIXEL(1, 0, 0) - GET_PIXEL(-1, 0, 0));
            fDy = 0.5 * (GET_PIXEL(0, 1, 0) - GET_PIXEL(0, -1, 0));
            fDz = 0.5 * (GET_PIXEL(0, 0, 1) - GET_PIXEL(0, 0, -1));
            afDerivativeMatrix[0][3] = -fDx;
            afDerivativeMatrix[1][3] = -fDy;
            afDerivativeMatrix[2][3] = -fDz;

            /* calculate the second order gradient */
            afDerivativeMatrix[0][0] = fDxx = GET_PIXEL(1, 0, 0) + GET_PIXEL(-1, 0, 0) - 2.0 * GET_PIXEL(0, 0, 0);
            afDerivativeMatrix[1][1] = fDyy = GET_PIXEL(0, 1, 0) + GET_PIXEL(0, -1, 0) - 2.0 * GET_PIXEL(0, 0, 0);
            afDerivativeMatrix[2][2] = fDzz = GET_PIXEL(0, 0, 1) + GET_PIXEL(0, 0, -1) - 2.0 * GET_PIXEL(0, 0, 0);

            afDerivativeMatrix[0][1] = afDerivativeMatrix[1][0] = \
                (fDxy = 0.25 * (GET_PIXEL(1, 1, 0) + GET_PIXEL(-1, -1, 0) - GET_PIXEL(-1, 1, 0) - GET_PIXEL(1, -1, 0)));
            afDerivativeMatrix[0][2] = afDerivativeMatrix[2][0] = \
                (fDxz = 0.25 * (GET_PIXEL(1, 0, 1) + GET_PIXEL(-1, 0, -1) - GET_PIXEL(-1, 0, 1) - GET_PIXEL(1, 0, -1)));
            afDerivativeMatrix[1][2] = afDerivativeMatrix[2][1] = \
                (fDyz = 0.25 * (GET_PIXEL(0, 1, 1) + GET_PIXEL(0, -1, -1) - GET_PIXEL(0, 1, -1) - GET_PIXEL(0, -1, 1)));
        
            /* When the gradient is 0, the real peak is there
                | Dxx Dxy Dxz |   | x |   | -Dx |
                | Dxy Dyy Dyz | * | y | = | -Dy |
                | Dxz Dyz Dzz |   | z |   | -Dz |
               use the matrix to calculate the x, y, z */
            this->gaussianElimination(afDerivativeMatrix, afResult);
        
            /* move the peak point */
            iOffsetX = ((0.6 < afResult[0] && iX < uiWidth  - 2) ? 1 : 0) + ((-0.6 > afResult[0] && iX > 1) ? -1 : 0);
            iOffsetY = ((0.6 < afResult[1] && iY < uiHeight - 2) ? 1 : 0) + ((-0.6 > afResult[1] && iY > 1) ? -1 : 0);
        
            if (0 == iOffsetX && 0 == iOffsetY)
            {
                break;
            }
        }

        /* calculaet the peak value */
        fPeakScore = GET_PIXEL(0, 0, 0) + 0.5 *(fDx * afResult[0] + fDy * afResult[1] + fDz * afResult[2]);
        fGamer = ((fDxx + fDyy) * (fDxx + fDyy)) / (fDxx * fDyy - fDxy *fDxy);

        /* delete the bad peak point */
        if (1.5 > fabs(afResult[0]) && 1.5 > fabs(afResult[1]) && 1.5 > fabs(afResult[2]) &&
            0   <= iX + afResult[0] && iX + afResult[0] <= uiWidth  - 1 &&
            0   <= iY + afResult[1] && iY + afResult[1] <= uiHeight - 1 &&
            0   <= iZ + afResult[2] && iZ + afResult[2] <= uiDepth  - 1 &&
            fabs(fPeakScore) > this->fPeakThreshold &&
            fGamer < (this->fEdgeThreshold + 1) * (this->fEdgeThreshold + 1) / this->fEdgeThreshold)
        {
            fSigma = 1.6 * pow(2.0, (iZ + afResult[2] + 1.0) / uiOctaveResolution);

            pstFeature->setFeaturePos(iX + afResult[0], iY + afResult[1]);

            pstFeature->setSigma(fSigma);

            iter++;
        }
        else
        {
            delete pstFeature;
            iter = this->vFeatures.erase(iter);
        }
    }
    return;
}

/* delete all features */
void DS_FEATURE_FILTER::deleteFeatures(void)
{
    DS_FEATURE *pstFeature = NULL;
    std::vector <DS_FEATURE *>::iterator iter;

    for (iter = this->vFeatures.begin(); iter != this->vFeatures.end(); )
    {
        pstFeature = *iter;
        delete pstFeature;
        iter = this->vFeatures.erase(iter);
    }

    return;
}

/* guassian eliminate the augmented matrix */
void DS_FEATURE_FILTER::gaussianElimination(float afAugmentedMatrix[3][4], float *pfResult)
{
    unsigned int uiLoopJ;
    unsigned int uiLoopI;
    int          iLoop;
    unsigned int uiLoopII;
    unsigned int uiLoopJJ;
    float        fMaxNum;
    float        fMaxAbsNum;
    unsigned int uiMaxI = 0;
    float        fTmp;

    for(uiLoopJ = 0 ; uiLoopJ < 3 ; ++uiLoopJ) 
    {
        fMaxNum = 0.0;
        fMaxAbsNum = 0.0;
        uiMaxI = 0;

        /* find the max point in uiLoopJ column */
        for (uiLoopI = uiLoopJ ; uiLoopI < 3 ; ++uiLoopI) 
        {
            if (fabs(afAugmentedMatrix[uiLoopI][uiLoopJ]) > fMaxAbsNum) 
            {
                fMaxNum = afAugmentedMatrix[uiLoopI][uiLoopJ];
                fMaxAbsNum = fabs(afAugmentedMatrix[uiLoopI][uiLoopJ]);
                uiMaxI = uiLoopI;
            }
        }

        /* when it is singular matrix return 0 */
        if (fMaxAbsNum < 1e-10)
        {
            memset(pfResult, 0, 3 * sizeof(float));
            return;
        }

        /* exchange the uiLoopJ row with the max row and normalize uiLoopJ row */
        for(uiLoopJJ = uiLoopJ ; uiLoopJJ < 4 ; ++uiLoopJJ)
        {
            fTmp = afAugmentedMatrix[uiMaxI][uiLoopJJ];
            afAugmentedMatrix[uiMaxI][uiLoopJJ] = afAugmentedMatrix[uiLoopJ][uiLoopJJ];
            afAugmentedMatrix[uiLoopJ][uiLoopJJ] = fTmp ;
            afAugmentedMatrix[uiLoopJ][uiLoopJJ] /= fMaxNum ;
        }

        /* from top to buttom delete all the elements buttom left */
        for (uiLoopII = uiLoopJ + 1 ; uiLoopII < 3 ; ++uiLoopII)
        {
            fTmp = afAugmentedMatrix[uiLoopII][uiLoopJ];
            for (uiLoopJJ = uiLoopJ ; uiLoopJJ < 4 ; ++uiLoopJJ)
            {
                afAugmentedMatrix[uiLoopII][uiLoopJJ] -= fTmp * afAugmentedMatrix[uiLoopJ][uiLoopJJ] ;
            }
        }

    }

    /* from buttom to top delete all the elements up right */
    for (uiLoopI = 2 ; uiLoopI != 0 ; --uiLoopI) 
    {
        for (iLoop = (int)uiLoopI - 1 ; iLoop >= 0 ; --iLoop)
        {
            fTmp = afAugmentedMatrix[iLoop][uiLoopI];
            afAugmentedMatrix[iLoop][3] -= fTmp * afAugmentedMatrix[uiLoopI][3];
        }
    }

    /* copy the calculation result */
    for (uiLoopI = 0; uiLoopI < 3; uiLoopI ++)
    {
        pfResult[uiLoopI] = afAugmentedMatrix[uiLoopI][3];
    }

    return;
}

/* calculate the key points */
void DS_FEATURE_FILTER::calcKeypoints(void)
{
    DS_FEATURE  *pstFeature = NULL;
    std::vector <DS_FEATURE *>::iterator iter;
    int          iOctave;
    int          iLevel;
    float        fLevel;
    SCALE_LEVEL *pstCurOctave = NULL;
    float       *pfGradGauss = NULL;

    for (iter = this->vFeatures.begin(); iter != this->vFeatures.end(); )
    {
        pstFeature = *iter;

        iOctave = pstFeature->getOctave();
        pstFeature->getFeaturePos(NULL, NULL, &fLevel);
        iLevel = fLevel;

        /* get the gaussian image */
        pstCurOctave = this->pstScaleSpace->getGradGaussData(iOctave);

        /* get the gradient gaussian */
        pfGradGauss = pstCurOctave->getLevelData(iLevel);
        
        /* calculate the main orientations */
        this->calcOrientations(pstFeature, pfGradGauss);

        /* calculate the feature vectors */
        this->calcFeatures(pstFeature, pfGradGauss);

        iter ++;
    }

    return;
}

/* calculate the main orientations */
void DS_FEATURE_FILTER::calcOrientations(DS_FEATURE *pstFeature, float *pfGradGauss)
{
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiStrideX;
    unsigned int uiStrideY;
    unsigned int uiLoop;
    int          iWindowSize;
    int          iOffsetX;
    int          iOffsetY;
    int          iPosX;
    int          iPosY;
    int          iMaxX;
    int          iMinX;
    int          iMaxY;
    int          iMinY;
    float        fPosX;
    float        fPosY;
    float        fSigma;
    float        fModulation;
    float        fAngle;
    float        fWeight;
    float       *pfImage = NULL;
    float        afHist[36];
    float        fMax;
    float        fOrientation;

    pstFeature->getFeaturePos(&fPosX, &fPosY, NULL);
    iPosX = fPosX;
    iPosY = fPosY;
    uiHeight = pstFeature->getHeight();
    uiWidth  = pstFeature->getWidth();
    uiStrideX = 2;
    uiStrideY = 2 * uiWidth;
    pfImage   = pfGradGauss + iPosX * uiStrideX + iPosY * uiStrideY;

    /* calculate the window size */
    fSigma = pstFeature->getSigma();
    iWindowSize = sqrt(2.0) * 3.0 * fSigma * 5.0 / 2.0;

    memset(afHist, 0, sizeof(afHist));

    iMaxX = DS_MIN(iWindowSize, uiWidth - iPosX -2);
    iMinX = DS_MAX(-iWindowSize, 1 - iPosX);
    iMaxY = DS_MIN(iWindowSize, uiHeight - iPosY -2);
    iMinY = DS_MAX(-iWindowSize, 1 - iPosY);

    /* for each all the directions and get 36-bin hist */
    for (iOffsetY = iMinY; iOffsetY <= iMaxY; iOffsetY ++)
    {
        for (iOffsetX = iMinX; iOffsetX <= iMaxX; iOffsetX ++)
        {
            fModulation = *(pfImage + iOffsetX * (int)uiStrideX + iOffsetY * (int)uiStrideY);
            fAngle      = *(pfImage + iOffsetX * (int)uiStrideX + iOffsetY * (int)uiStrideY + 1);
            
            fWeight = exp(- (iOffsetX * iOffsetX + iOffsetY * iOffsetY) / (2 * fSigma * fSigma));

            afHist[(int)(18.0 * fAngle / DS_PI)] += fWeight * fModulation;
        }
    }

    /* find the max point */
    fMax = 0.0;
    for (uiLoop = 0; uiLoop < 36; uiLoop ++)
    {
        if (afHist[uiLoop] > fMax)
        {
            fMax = afHist[uiLoop];
        }
    }

    /* find the point larger than 80% max, and larger than its neighbour */
    for (uiLoop = 0; uiLoop < 36; uiLoop ++)
    {
        if (afHist[uiLoop] > 0.8 * fMax &&
            afHist[uiLoop] > afHist[(uiLoop + 37) % 36] &&
            afHist[uiLoop] > afHist[(uiLoop + 35) % 36])
        {
            fOrientation = 2.0 * DS_PI * uiLoop / 36;
            pstFeature->addOrientation(fOrientation);
        }
    }

    return;
}

/* calculate feature vectors */
void DS_FEATURE_FILTER::calcFeatures(DS_FEATURE *pstFeature, float *pfGradGauss)
{
    unsigned int uiOrientationNum;
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiLoop;
    unsigned int uiStrideX;
    unsigned int uiStrideY;
    int          iWindowSize;
    int          iOffsetX;
    int          iOffsetY;
    int          iPosX;
    int          iPosY;
    int          iPosZ;
    int          iMaxX;
    int          iMinX;
    int          iMaxY;
    int          iMinY;
    float        fPosX;
    float        fPosY;
    float        fSigma;
    float        fModulation;
    float        fAngle;
    float        fDAngle;
    float        afOrientations[4];
    float       *pfImage = NULL;
    float        fCosOrientation;
    float        fSinOrientation;
    float        fWeight;
    float        afFeature[128];
    float       *pfFeature = NULL;

    pstFeature->getFeaturePos(&fPosX, &fPosY, NULL);
    iPosX = fPosX;
    iPosY = fPosY;
    uiHeight = pstFeature->getHeight();
    uiWidth  = pstFeature->getWidth();
    uiStrideX = 2;
    uiStrideY = 2 * uiWidth;
    pfImage   = pfGradGauss + iPosX * (int)uiStrideX + iPosY * (int)uiStrideY;

    /* calculate window size */
    fSigma = pstFeature->getSigma();
    iWindowSize = sqrt(2.0) * 3.0 * fSigma * 5.0 / 2.0;

    /* get the orientation number */
    uiOrientationNum = pstFeature->getOrientationNum();

    /* get all orientations */
    pstFeature->getOrientation(afOrientations);

    iMaxX = DS_MIN(iWindowSize, uiWidth - iPosX -2);
    iMinX = DS_MAX(-iWindowSize, 1 - iPosX);
    iMaxY = DS_MIN(iWindowSize, uiHeight - iPosY -2);
    iMinY = DS_MAX(-iWindowSize, 1 - iPosY);


    for (uiLoop = 0; uiLoop < uiOrientationNum; uiLoop ++)
    {
        fCosOrientation = cos(afOrientations[uiLoop]);
        fSinOrientation = sin(afOrientations[uiLoop]);

        memset(afFeature, 0, sizeof(afFeature));
        pfFeature = afFeature + 80;
        for (iOffsetY = iMinY; iOffsetY <= iMaxY; iOffsetY ++)
        {
            for (iOffsetX = iMinX; iOffsetX <= iMaxX; iOffsetX ++)
            {
                fModulation = *(pfImage + iOffsetX * (int)uiStrideX + iOffsetY * (int)uiStrideY);
                fAngle      = *(pfImage + iOffsetX * (int)uiStrideX + iOffsetY * (int)uiStrideY + 1);

                fDAngle     = (fAngle > afOrientations[uiLoop] ? (fAngle - afOrientations[uiLoop]) : (fAngle - afOrientations[uiLoop] + 2.0 * DS_PI));

                iPosX = ( fCosOrientation * iOffsetX + fSinOrientation * iOffsetY) / (3.0 * fSigma);
                iPosY = (-fSinOrientation * iOffsetX + fCosOrientation * iOffsetY) / (3.0 * fSigma);
                iPosZ = 8.0 * fDAngle / (2.0 * DS_PI);
                
                fWeight = exp(- (iPosX * iPosX + iPosY * iPosY) / 8.0);

                if (-2 <= iPosX && iPosX < 2 && 
                    -2 <= iPosY && iPosY < 2)
                {
                    *(pfFeature + iPosZ + 32 * iPosY + 8 * iPosX) += fModulation * fWeight;
                }
            }
        }

        this->uiFeatureNum ++;
        pstFeature->setFeature(afFeature, uiLoop);
    }

    /* normalize all features */
    pstFeature->normalizeFeature();

    return;
}

