#ifndef _DS_SCALESPACE_H_
#define _DS_SCALESPACE_H_

class SCALE_LEVEL
{
public:
    SCALE_LEVEL(unsigned int uiWidth, unsigned int uiHeight, int iFirstLevel, int iLastLevel);
    virtual ~SCALE_LEVEL(void);
private:
    unsigned int   uiWidth;
    unsigned int   uiHeight;
    int            iFirstLevel;
    int            iLastLevel;
    unsigned int   uiLevelNum;

    /* point to a buffer storing all the image of the same scale */
    float         *pfGaussImages;

public:
    /* set the data of a level */
    void setLevelData(float *pfImgData, int iLevel);

    /* get the data of a level */
    float *getLevelData(int iLevel);

    /* get all the data */
    float *getData(void);

    /* create all levels */
    int createLevels(void);

    /* get image height */
    unsigned int getHeight(void);

    /* get image width */
    unsigned int getWidth(void);

    /* get image level number */
    unsigned int getLevelNum(void);
};

class DS_SCALESPACE
{
public:
    DS_SCALESPACE(DS_PGM *pstPic, int iFirstOctave, int iLastOctave, int iFirstLevel, int iLastLevel, unsigned int uiOctaveResolution);
    virtual ~DS_SCALESPACE(void);
private:
    double dSigma0;
    double dSigmaN;
    double dSigmaK;
    double dDSigma0;
    int    iFirstOctave;
    int    iLastOctave;
    int    iFirstLevel;
    int    iLastLevel;
    unsigned int uiOctaveNum;
    unsigned int uiOctaveResolution;
    DS_PGM *pstPic;

    /* each element contains all the gaussian smooth image in the same scale */
    std::vector <SCALE_LEVEL *> vGaussOctaves;

    /* each element contains all the DoG image in the same scale */
    std::vector <SCALE_LEVEL *> vGoDOctaves;

    /* the gradient and directions */
    std::vector <SCALE_LEVEL *> vGradGaussOctaves;

public:
    /* create the memory used for the scale space */
    int createScaleSpace(void);

    /* get the DoG */
    SCALE_LEVEL *getDoGData(int iOctave);

    /* get gaussian image */
    SCALE_LEVEL *getGaussData(int iOctave);
    
    /* get gradient */
    SCALE_LEVEL *getGradGaussData(int iOctave);

    /* get first octave */
    int getFirstOctave(void);

    /* get last octave */
    int getLastOctave(void);

    /* get first level */
    int getFirstLevel(void);

    /* get last level */
    int getLastLevel(void);

    /* get octave resolution */
    int getOctaveResolution(void);
private:
    /* get the size of octave */
    void getSizeByOctave(unsigned int * puiWidth, unsigned int * puiHeight, int iOctave);

    /* delete all space */
    void deleteScaleSpace(void);

    /* up sample image */
    void upSampleImage(float *pdSrcImg, float *pfUpImage, unsigned int uiWidth, unsigned int uiHeight);

    /* down sample image */
    void downSampleImage(float *pfSrcImg, float *pfDownImage, unsigned int uiWidth, unsigned int uiHeight);
    
    /* gaussian smooth on image */
    int gaussSmooth(float *pfSrcImg, float *pfDesImg, float fSigma, unsigned int uiWidth, unsigned int uiHeight);

    /* create gaussian filter */
    float *createGuassFilter(unsigned int *puiFilterSize, float fSigma);
    
    /* run gaussian horizonal */
    void convolveHeight(float        *pfFilter, 
                        unsigned int  uiFilterSize,
                        float        *pfDesImg, 
                        float        *pfSrcImg,
                        unsigned int  uiWidth, 
                        unsigned int  uiHeight);

    /* run gaussian vertical */
    void convolveWidth(float        *pfFilter, 
                       unsigned int  uiFilterSize,
                       float        *pfDesImg, 
                       float        *pfSrcImg,
                       unsigned int  uiWidth, 
                       unsigned int  uiHeight);
    
    /* fill gaussian space */
    int fillGauseSpace(void);

    /* fill DoG space */
    void fillDoGSpace(void);

    /* fill gradient gaussian space*/
    void fillGradGaussSpace(void);

    /* calculate the gradient and direction */
    void calcCurGradient(float *pfGauss, float *pfGradGauss, unsigned int uiWidth, unsigned int uiHeight);

    /* calculate the sigma of current octave and level */
    double calculateSigma(int iOctave, int iLevel);
};

#endif
