#include <iostream>
#include "ds_main.h"
#include "ds_pgm.h"
#include "ds_debug.h"

/* write file to path */
int DS_WriteImage(char *pcFilePath, float *pfImage, unsigned int uiWidth, unsigned int uiHeight)
{
    int iRet = 0;
    int iValue;
    FILE *pFile = NULL;
    unsigned int uiLoopI;
    unsigned int uiLoopJ;

    pFile = fopen(pcFilePath, "wb");
    if (NULL == pFile)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to create image %s.", pcFilePath);
        return -1;
    }

    fprintf(pFile, "P5\n%d %d\n255\n", uiWidth, uiHeight);

    for (uiLoopI = 0; uiLoopI < uiHeight; uiLoopI ++)
    {
        for (uiLoopJ = 0; uiLoopJ < uiWidth; uiLoopJ ++)
        {
            iValue = *(pfImage + uiLoopJ + uiLoopI * uiWidth);
            fputc(DS_MAX(0, DS_MIN(iValue, 255)), pFile);
        }
    }

    fclose(pFile);

    return iRet;
}

DS_PGM::DS_PGM(char *pcFilePath)
    : uiWidth(0)
    , uiHeight(0)
    , uiMaxValue(0)
    , pfImageData(NULL)
    , bIsRaw(false)

{
    strcpy(szFilePath, pcFilePath);
}

DS_PGM::~DS_PGM(void)
{
    if (NULL != this->pfImageData)
    {
        delete this->pfImageData;
    }
    
    return;
}

/* remove the blank in the file */
int DS_PGM::removeBlanks(FILE *pFile)
{
    int iBlankcount = 0;
    int iTmpChar;
    int iIsContinue = 1;

    while (1 == iIsContinue)
    {
        iTmpChar = fgetc(pFile);

        switch(iTmpChar) 
        {
            case '\t':
            case '\n':
            case '\r':
            case ' ' :
            case '#' :
            {
                iBlankcount++;
                break;           
            }
            case EOF:
            {
                iIsContinue = 0;
                break;
            }
            default:
            {
                ungetc(iTmpChar, pFile);
                iIsContinue = 0;
                break;
            }
        }
    }

    return iBlankcount;
}

/* read the head of file */
int DS_PGM::extractPgmHead(FILE *pFile)
{
    char acFileType[2];
    bool  bIsRawType;
    unsigned int uiMaxValue;
    unsigned int uiWidth;
    unsigned int uiHeight;

    if ((2 > fread(acFileType, 1, 2, pFile)) || ('P' != acFileType[0])) 
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to read file %s.", this->szFilePath);
        return -1;
    }

    /* the first two bytes can only be "P2" or "P5" */
    if ('2' == acFileType[1])
    {
        bIsRawType = false;
    }
    else if ('5' == acFileType[1])
    {
        bIsRawType = true;
    }
    else
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    /* jump over the blank */
    if (0 == this->removeBlanks(pFile))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    /* read image width */
    if (1 != fscanf(pFile, "%d", &uiWidth))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse width, file %s.", this->szFilePath);
        return -1;
    }

    /* jump over the blank */
    if (0 == this->removeBlanks(pFile))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    /* read image height */
    if (1 != fscanf(pFile, "%d", &uiHeight))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse height, file %s.", this->szFilePath);
        return -1;
    }

    /* jump over the blank */
    if (0 == this->removeBlanks(pFile))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    if (1 != fscanf(pFile, "%d", &uiMaxValue))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    /* must end with a single blank */
    if (0 == this->removeBlanks(pFile))
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to parse file %s.", this->szFilePath);
        return -1;
    }

    if(65536 < uiMaxValue) 
    {
        return -1;
    }

    this->uiMaxValue = uiMaxValue;
    this->uiHeight = uiHeight;
    this->uiWidth  = uiWidth;
    this->bIsRaw   = bIsRawType;

    return 0;
}

/* get size of single pixel */
unsigned int DS_PGM::getPixelSize(void)
{
    return (this->uiMaxValue >= 256) + 1;
}

/* get image size (pixel number) */
unsigned int DS_PGM::getImageSize(void)
{
    return this->uiHeight * this->uiWidth;
}

/* extract image data */
int DS_PGM::extractPgmData(FILE* pFile)
{
    size_t  ulReadSize = 0;
    int     iRet = 0;
    unsigned int uiPixelSize;
    unsigned int uiDataSize;
    unsigned int uiLoop;
    unsigned char ucTmp;
    uiPixelSize = this->getPixelSize();
    uiDataSize  = this->getImageSize();

    for(uiLoop = 0; uiLoop < uiDataSize; uiLoop++) 
    {
        ucTmp = 0;
        ulReadSize = fread(&ucTmp, uiPixelSize, 1, pFile);
        if (uiPixelSize != ulReadSize)
        {
            iRet = -1;
            break;
        }

        *(this->pfImageData + uiLoop) = ucTmp;
    }

    return iRet;
}

/* get image data */
float *DS_PGM::getImageData(void)
{
    return this->pfImageData;
}

/* get image height */
unsigned int DS_PGM::getHeight(void)
{
    return this->uiHeight;
}

/* get image height */
unsigned int DS_PGM::getWidth(void)
{
    return this->uiWidth;
}

/* create image based on path */
int DS_PGM::createImage(void)
{
    FILE *pFile = NULL;
    int   iRet = 0;
    unsigned int uiPixelSize;

    pFile = fopen(this->szFilePath, "rb");
    if (NULL == pFile)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to open file %s.", this->szFilePath);
        return -1;
    }

    /* extract file header */
    iRet = this->extractPgmHead(pFile);
    if (0 != iRet)
    {
        fclose(pFile);
        return iRet;
    }

    /* malloc image data */
    uiPixelSize = this->getPixelSize();
    this->pfImageData = new float[this->uiWidth * this->uiHeight];
    if (NULL == this->pfImageData)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to alloc memery.");
        fclose(pFile);
        return -1;
    }

    /* extract image data */
    iRet = this->extractPgmData(pFile);
    if (0 != iRet)
    {
        DS_WriteErrorInfo((char *)__FILE__, __LINE__, (char *)"Failed to extract data from file %s.", this->szFilePath);
        delete this->pfImageData;
    }

    DS_WriteDebugInfo((char *)"read file %s successfully, file size is %d*%d.", this->szFilePath, this->uiWidth, this->uiHeight);

    fclose(pFile);

    return iRet;
}
