#ifndef _DS_PGM_H_
#define _DS_PGM_H_

/* write image to file */
int DS_WriteImage(char *pcFilePath, float *pfImage, unsigned int uiWidth, unsigned int uiHeight);

class DS_PGM
{
public:
    DS_PGM(char *pcFilePath);
    virtual ~DS_PGM(void);

private:
    unsigned int   uiWidth;    
    unsigned int   uiHeight;
    unsigned int   uiMaxValue;
    float         *pfImageData;
    bool           bIsRaw;
    char           szFilePath[256];

    /* jump over the blanks */
    int removeBlanks(FILE *pFile);

    /* parse the file header */
    int extractPgmHead(FILE *pFile);

    /* get the size of one pixel */
    unsigned int getPixelSize(void);

    /* parse image data */
    int extractPgmData(FILE* pFile);

public:
    /* get image size */
    unsigned int getImageSize(void);

    /* get image data */
    float *getImageData(void);

    /* get image height */
    unsigned int getHeight(void);

    /* get image width */
    unsigned int getWidth(void);

    /* create image */
    int createImage(void);
};

#endif
