#include <iostream>
#include <vector>
#include <math.h>
#include "ds_main.h"
#include "ds_debug.h"
#include "ds_pgm.h"
#include "ds_scalespace.h"
#include "ds_feature.h"
#include "ds_sift.h"
#include "ds_match.h"


DS_MATCH::DS_MATCH(DS_SIFT *pstFirst, DS_SIFT *pstSecond)
{
    this->pstFirst  = pstFirst;
    this->pstSecond = pstSecond;

    return;
}

DS_MATCH::~DS_MATCH(void)
{
    this->deleteMatchedPoint();
}

/* match image */
int DS_MATCH::matchPictures(void)
{
    int          iRet = 0;
    unsigned int uiLoopFirst;
    unsigned int uiLoopSecond;
    double       dDistance1;
    double       dDistance2;
    double       dDistance;
    DS_FEATURE *pstFirst  = NULL;
    DS_FEATURE *pstTmp    = NULL;
    DS_FEATURE *pstSecond = NULL;
    std::vector <DS_FEATURE *> *pvFeatureFirst  = NULL;
    std::vector <DS_FEATURE *> *pvFeatureSecond = NULL;
    std::vector <DS_FEATURE *>::iterator iterFirst;
    std::vector <DS_FEATURE *>::iterator iterSecond;
    MATCHED_POINT *pstMatchedPonit = NULL;

    /* get the features of two images */
    pvFeatureFirst  = this->pstFirst->getAllFeatures();
    pvFeatureSecond = this->pstSecond->getAllFeatures();

    for (iterFirst = pvFeatureFirst->begin(); iterFirst != pvFeatureFirst->end(); iterFirst ++)
    {   
        pstFirst = *iterFirst;
        for (uiLoopFirst = 0; uiLoopFirst < pstFirst->getOrientationNum(); uiLoopFirst ++)
        {
            dDistance1 = 100000000.0;
            dDistance2 = 100000000.0;

            for (iterSecond = pvFeatureSecond->begin(); iterSecond != pvFeatureSecond->end(); iterSecond ++)
            {
                pstSecond = *iterSecond;

                for (uiLoopSecond = 0; uiLoopSecond < pstSecond->getOrientationNum(); uiLoopSecond ++)
                {
                    dDistance = calcDistance(pstFirst->getFeature(uiLoopFirst), pstSecond->getFeature(uiLoopSecond));
                    if (dDistance < dDistance1)
                    {
                        dDistance2 = dDistance1;
                        dDistance1 = dDistance;
                        pstTmp = pstSecond;
                    }
                    else if (dDistance < dDistance2)
                    {
                        dDistance2 = dDistance;
                    }
                }
            }

            if (2.0 * dDistance1 < dDistance2)
            {
                /* when matched saved then into list */
                pstMatchedPonit = new MATCHED_POINT();
                if (NULL == pstMatchedPonit)
                {
                    iRet = -1;
                    break;
                }
                pstFirst->getRealFeaturePos(&(pstMatchedPonit->uiPosXFirst), &(pstMatchedPonit->uiPosYFirst));
                pstTmp->getRealFeaturePos(&(pstMatchedPonit->uiPosXSecond), &(pstMatchedPonit->uiPosYSecond));
                pstMatchedPonit->bIsSample = false;

                this->vMatchedPoint.push_back(pstMatchedPonit);
            }
        }

        if(0 != iRet)
        {
            break;
        }
    }

    if (0 != iRet)
    {
        this->deleteMatchedPoint();
    }

    DS_WriteDebugInfo((char *)"%d points are matched!", this->vMatchedPoint.size());

    return iRet;
}

/* draw the match result */
int DS_MATCH::drawResult(char *pcFilePath)
{
    int          iRet = 0;
    unsigned int uiLoop;
    unsigned int uiLoopI;
    unsigned int uiLoopJ;
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiWidthFirst;
    unsigned int uiHeightFirst;
    unsigned int uiWidthSecond;
    unsigned int uiHeightSecond;
    float       *pfImage = NULL;
    float       *pfTmp = NULL;

    uiWidthFirst   = this->pstFirst->getWidth();
    uiHeightFirst  = this->pstFirst->getHeight();
    uiWidthSecond  = this->pstSecond->getWidth();
    uiHeightSecond = this->pstSecond->getHeight();
    uiWidth = uiWidthFirst  + uiWidthSecond;
    uiHeight = uiHeightFirst + uiHeightSecond;

    pfImage = new float[uiWidth * uiHeight];
    if (NULL == pfImage)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create image buffer.");
        return -1;
    }

    /* set the blank into gray */
    pfTmp = pfImage;
    for (uiLoop = 0; uiLoop < uiWidth * uiHeight; uiLoop ++)
    {
        *pfTmp = 128.0;
        pfTmp++;
    }

    /* save the first image into buffer */
    pfTmp = this->pstFirst->getImageData();
    for (uiLoopI = 0; uiLoopI < uiWidthFirst; uiLoopI ++)
    {
        for (uiLoopJ = 0; uiLoopJ < uiHeightFirst; uiLoopJ ++)
        {
            *(pfImage + uiLoopI + uiLoopJ * uiWidth) = *(pfTmp + uiLoopI + uiLoopJ * uiWidthFirst);
        }
    }

    /* save the second image into buffer */
    pfTmp = this->pstSecond->getImageData();
    for (uiLoopI = 0; uiLoopI < uiWidthSecond; uiLoopI ++)
    {
        for (uiLoopJ = 0; uiLoopJ < uiHeightSecond; uiLoopJ ++)
        {
            *(pfImage + uiLoopI + uiWidthFirst + uiLoopJ * uiWidth) = *(pfTmp + uiLoopI + uiLoopJ * uiWidthSecond);
        }
    }

    /* draw lines on the image */
    this->drawLine(pfImage);

    iRet = DS_WriteImage(pcFilePath, pfImage, uiWidth, uiHeight);

    delete pfImage;

    return iRet;
}

/* calculate the disrances of two images */
double DS_MATCH::calcDistance(float *pfFirst, float *pfSecond)
{
    unsigned int uiLoop;
    double       dDistance = 0.0;
    float        fDistance;

    for (uiLoop = 0 ; uiLoop < 128; uiLoop ++)
    {
        fDistance = pfFirst[uiLoop] - pfSecond[uiLoop];

        dDistance += fDistance * fDistance;
    }

    return dDistance;
}

/* delete all matched points */
void DS_MATCH::deleteMatchedPoint(void)
{
    MATCHED_POINT *pstCurPoint = NULL;
    std::vector <MATCHED_POINT *>::iterator iter;

    for (iter = this->vMatchedPoint.begin(); iter != this->vMatchedPoint.end(); )
    {
        pstCurPoint = *iter;
        delete pstCurPoint;
        iter = this->vMatchedPoint.erase(iter);
    }

    return ;
}

/* draw linw on the buffer image */
void DS_MATCH::drawLine(float *pfImage)
{
    unsigned int uiLoop;
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiWidthFirst;
    unsigned int uiHeightFirst;
    unsigned int uiWidthSecond;
    unsigned int uiHeightSecond;
    int          iDisWidth;
    int          iDisHeight;

    uiWidthFirst   = this->pstFirst->getWidth();
    uiHeightFirst  = this->pstFirst->getHeight();
    uiWidthSecond  = this->pstSecond->getWidth();
    uiHeightSecond = this->pstSecond->getHeight();
    uiWidth = uiWidthFirst  + uiWidthSecond;
    uiHeight = uiHeightFirst + uiHeightSecond;

    MATCHED_POINT *pstCurPoint = NULL;
    std::vector <MATCHED_POINT *>::iterator iter;

    for (iter = this->vMatchedPoint.begin(); iter != this->vMatchedPoint.end(); iter++)
    {
        pstCurPoint = *iter;

        iDisWidth  = (int)pstCurPoint->uiPosXSecond + (int)uiWidthFirst - (int)pstCurPoint->uiPosXFirst;
        iDisHeight = (int)pstCurPoint->uiPosYSecond - (int)pstCurPoint->uiPosYFirst;

        for (uiLoop = pstCurPoint->uiPosXFirst; uiLoop < pstCurPoint->uiPosXSecond + uiWidthFirst; uiLoop ++)
        {
            pfImage[(int)uiLoop + ((int)(uiLoop - pstCurPoint->uiPosXFirst) * iDisHeight / iDisWidth + (int)pstCurPoint->uiPosYFirst) * uiWidth] = 255.0;
        }
    }

    return;
}

/* get matched number */
unsigned int DS_MATCH::getMatchedNum(void)
{
    return this->vMatchedPoint.size();
}

/* ranasc eliminate the mis matched point */
void DS_MATCH::ransacFilter(void)
{
    int iRet = 0;
    unsigned int uiLoop = 0;
    MATCHED_POINT *pstCurPoint = NULL;
    std::vector <MATCHED_POINT *> vSample;
    std::vector <MATCHED_POINT *> vTmpSample;
    std::vector <MATCHED_POINT *>::iterator iter;
    float        afMatrix[6];
    float        fErrorRatio;

    /* when the matched point is less than 20, do not ransac */
    if (20 > this->vMatchedPoint.size())
    {
        return;
    }

    while (1000 > uiLoop)
    {
        /* select two points */
        this->ransacSample(&vTmpSample);
    
        /* calculate the ranasc matrix */
        iRet = this->ransacCalcMatrix(&vTmpSample, afMatrix);
        if (0 != iRet)
        {
            iRet = 0;
            continue;
        }

        /* remove all the sample point, will be added latter */
        vTmpSample.clear();
    
        /* add the points to the set */
        this->ransacAddtoModel(&vTmpSample, afMatrix);
    
        /* when 99% points is in the set */
        fErrorRatio = 1.0 - (float)vTmpSample.size() / (float)this->vMatchedPoint.size();
        if (1.0 - pow(fErrorRatio, 3) > 0.99)
        {
            this->ransacCopySample(&(vSample), &(vTmpSample));
            break;
        }

        /* when the temp set is larger than current replace it */
        if (vTmpSample.size() > vSample.size())
        {
            this->ransacCopySample(&(vSample), &(vTmpSample));
        }

        uiLoop ++;
    }

    /* delete the points no tin the set from the original matches */
    for (iter = this->vMatchedPoint.begin(); iter != this->vMatchedPoint.end(); )
    {
        pstCurPoint = *iter;
        if (false == pstCurPoint->bIsSample)
        {
            delete pstCurPoint;
            iter = this->vMatchedPoint.erase(iter);
        }
        else
        {
            iter ++;
        }
    }

    DS_WriteDebugInfo((char *)"%d points are finally matched after ransac!", this->vMatchedPoint.size());

    return;
}

/* copy the sample set */
void DS_MATCH::ransacCopySample(std::vector <MATCHED_POINT *> *pvDstSamlple, std::vector <MATCHED_POINT *> *pvSrcSamlple)
{
    MATCHED_POINT *pstCurPoint = NULL;
    std::vector <MATCHED_POINT *>::iterator iter;

    /* clear the destination set */
    for (iter = pvDstSamlple->begin(); iter != pvDstSamlple->end(); )
    {
        pstCurPoint = *iter;
        pstCurPoint->bIsSample = false;
        iter = pvDstSamlple->erase(iter);
    }

    for (iter = pvSrcSamlple->begin(); iter != pvSrcSamlple->end(); )
    {
        pstCurPoint = *iter;
        pstCurPoint->bIsSample = true;
        pvDstSamlple->push_back(pstCurPoint);
        iter = pvSrcSamlple->erase(iter);
    }
    return;
}

/* randomly sample all the features */
void DS_MATCH::ransacSample(std::vector <MATCHED_POINT *> *pvSamlple)
{
    MATCHED_POINT *pstCurPoint = NULL;
    unsigned int uiMatchedNum;
    unsigned int uiLoop;
    unsigned int uiPos;
    bool         bIsExist;
    uiMatchedNum = this->vMatchedPoint.size();
    std::vector <MATCHED_POINT *>::iterator iter;

    for (uiLoop = 0; uiLoop < 3; uiLoop ++)
    {
        uiPos = rand() % uiMatchedNum;
        
        /* check whether the point exists */
        bIsExist = false;
        for (iter = pvSamlple->begin(); iter != pvSamlple->end(); iter++)
        {
            pstCurPoint = *iter;
            if (pstCurPoint == this->vMatchedPoint[uiPos])
            {
                bIsExist = true;
                break;
            }
        }

        /* if it does not exist, then continue add */
        if (false == bIsExist)
        {
            pvSamlple->push_back(this->vMatchedPoint[uiPos]);
        }
        else
        {
            uiLoop --;
        }
    }

    return;
}

/* calculate ransac matrix */
int DS_MATCH::ransacCalcMatrix(std::vector <MATCHED_POINT *> *pvSamlple, float *pfMatrix)
{
    int iRet;

    float afMatrix1[3][4] = {{(float)((*pvSamlple)[0])->uiPosXFirst, (float)((*pvSamlple)[0])->uiPosYFirst, 1.0, (float)((*pvSamlple)[0])->uiPosXSecond},
                             {(float)((*pvSamlple)[1])->uiPosXFirst, (float)((*pvSamlple)[1])->uiPosYFirst, 1.0, (float)((*pvSamlple)[1])->uiPosXSecond},
                             {(float)((*pvSamlple)[2])->uiPosXFirst, (float)((*pvSamlple)[2])->uiPosYFirst, 1.0, (float)((*pvSamlple)[2])->uiPosXSecond}};

    float afMatrix2[3][4] = {{(float)((*pvSamlple)[0])->uiPosXFirst, (float)((*pvSamlple)[0])->uiPosYFirst, 1.0, (float)((*pvSamlple)[0])->uiPosYSecond},
                             {(float)((*pvSamlple)[1])->uiPosXFirst, (float)((*pvSamlple)[1])->uiPosYFirst, 1.0, (float)((*pvSamlple)[1])->uiPosYSecond},
                             {(float)((*pvSamlple)[2])->uiPosXFirst, (float)((*pvSamlple)[2])->uiPosYFirst, 1.0, (float)((*pvSamlple)[2])->uiPosYSecond}};

    iRet = this->ransacSolveMatrix(afMatrix1, &pfMatrix[0]);
    if (0 == iRet)
    {
        iRet = this->ransacSolveMatrix(afMatrix2, &pfMatrix[3]);
    }

    return iRet;
}

/* solve ransac matrix */
int DS_MATCH::ransacSolveMatrix(float afMatrix[3][4],  float *pfResult)
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

        /* find the max value in uiLoopJ column */
        for (uiLoopI = uiLoopJ ; uiLoopI < 3 ; ++uiLoopI) 
        {
            if (fabs(afMatrix[uiLoopI][uiLoopJ]) > fMaxAbsNum) 
            {
                fMaxNum = afMatrix[uiLoopI][uiLoopJ];
                fMaxAbsNum = fabs(afMatrix[uiLoopI][uiLoopJ]);
                uiMaxI = uiLoopI;
            }
        }

        /* when it is singular matrix, return 0 */
        if (fMaxAbsNum < 1e-10)
        {
            memset(pfResult, 0, 3 * sizeof(float));
            return -1;
        }

        /* change the uiLoopJ row with the max row and then normalize uiLoopJ row */
        for(uiLoopJJ = uiLoopJ ; uiLoopJJ < 4 ; ++uiLoopJJ)
        {
            fTmp = afMatrix[uiMaxI][uiLoopJJ];
            afMatrix[uiMaxI][uiLoopJJ] = afMatrix[uiLoopJ][uiLoopJJ];
            afMatrix[uiLoopJ][uiLoopJJ] = fTmp ;
            afMatrix[uiLoopJ][uiLoopJJ] /= fMaxNum ;
        }

        /* up-buttom minus, remove the left-down  */
        for (uiLoopII = uiLoopJ + 1 ; uiLoopII < 3 ; ++uiLoopII)
        {
            fTmp = afMatrix[uiLoopII][uiLoopJ];
            for (uiLoopJJ = uiLoopJ ; uiLoopJJ < 4 ; ++uiLoopJJ)
            {
                afMatrix[uiLoopII][uiLoopJJ] -= fTmp * afMatrix[uiLoopJ][uiLoopJJ] ;
            }
        }

    }

    /* buttom-up minus, remove the right-up */
    for (uiLoopI = 2 ; uiLoopI != 0 ; --uiLoopI) 
    {
        for (iLoop = (int)uiLoopI - 1 ; iLoop >= 0 ; --iLoop)
        {
            fTmp = afMatrix[iLoop][uiLoopI];
            afMatrix[iLoop][3] -= fTmp * afMatrix[uiLoopI][3];
        }
    }

    /* copy the calculate result */
    for (uiLoopI = 0; uiLoopI < 3; uiLoopI ++)
    {
        pfResult[uiLoopI] = afMatrix[uiLoopI][3];
    }

    return 0;
}

/* put the matched points into set */
void DS_MATCH::ransacAddtoModel(std::vector <MATCHED_POINT *> *pvSamlple, float *pfMatrix)
{
    MATCHED_POINT *pstCurPoint = NULL;
    std::vector <MATCHED_POINT *>::iterator iter;
    float fX;
    float fY;
    float fDistance;
    float fThreshold;

    /* calculate the threshold based on the image size */
    fThreshold = 0.01 * DS_MIN(this->pstFirst->getHeight() * this->pstFirst->getWidth(), 
                                 this->pstSecond->getHeight() *  this->pstSecond->getWidth());
    fThreshold = fThreshold * fThreshold;

    for (iter = this->vMatchedPoint.begin(); iter != this->vMatchedPoint.end(); iter ++)
    {
        pstCurPoint = *iter;
        fX = (float)pstCurPoint->uiPosXFirst * pfMatrix[0] + (float)pstCurPoint->uiPosYFirst * pfMatrix[1] + pfMatrix[2];
        fY = (float)pstCurPoint->uiPosXFirst * pfMatrix[3] + (float)pstCurPoint->uiPosYFirst * pfMatrix[4] + pfMatrix[5];

        fDistance = (fX - (float)pstCurPoint->uiPosXSecond) * (fX - (float)pstCurPoint->uiPosXSecond) + 
                    (fY - (float)pstCurPoint->uiPosYSecond) * (fY - (float)pstCurPoint->uiPosYSecond);

        if (fDistance < fThreshold)
        {
            pvSamlple->push_back(pstCurPoint);
        }
    }

    return;
}
