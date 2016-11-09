#ifndef _DS_FEATURE_H_
#define _DS_FEATURE_H_

class DS_FEATURE
{
public:
    DS_FEATURE(float fPosX, float fPosY, float fPosZ, int iOctave, unsigned int uiWidth, unsigned int uiHeight);
    ~DS_FEATURE(void);
private:
    float fPosX;
    float fPosY;
    float fPosZ;
    int   iOctave;
    unsigned int   uiWidth;
    unsigned int   uiHeight;
    unsigned int   uiOrientationNum;
    float  afOrientations[4];
    float  afFeature[4][128];
    float  fSigma;

public:
    /* get the octave number */
    int getOctave(void);

    /* get the width of the feature space */
    unsigned int getWidth(void);

    /* get the height of the feature space */
    unsigned int getHeight(void);

    /* get the position of the feature */
    void getFeaturePos(float *pfX, float *pfY, float *pfZ);

     /* get the position of the feature in real image */
    void getRealFeaturePos(unsigned int *puiX, unsigned int *puiY);

    /* set the position of the feature */
    void setFeaturePos(float fPosX, float fPosY);

    /* set the sigma value */
    void setSigma(float fSigma);

    /* get the sigma value */
    float getSigma(void);

    /* get the orientation number */
    unsigned int getOrientationNum(void);
    
    /* add an orientation */
    void addOrientation(float fOrientation);

    /* get the orientations */
    void getOrientation(float *pfOrientations);

    /* get the 128 dim feature */
    float *getFeature(unsigned int uiOrientationNum);

    /* add a 128 dim feature*/
    void setFeature(float *pfFeature, unsigned int uiOrientationNum);

    /* normalize feature */
    void normalizeFeature(void);

    /* transfer feature into 0~255 integer */
    void transFeatureToInt(void);
};

class DS_FEATURE_FILTER
{
public:
    DS_FEATURE_FILTER(DS_SCALESPACE *pstScaleSpace);
    ~DS_FEATURE_FILTER(void);
private:
    DS_SCALESPACE *pstScaleSpace;
    std::vector <DS_FEATURE *> vFeatures;
    float fPeakThreshold;
    float fEdgeThreshold;
    unsigned int uiFeatureNum;

public:
    /* create features */
    int createFeatures(void);

    /* transfer feature into 0~255 integer*/
    void transFeatureToInt(void);

    /* get all features */
    std::vector <DS_FEATURE *> *getAllFeatures(void);

    /* get features numbers */
    unsigned int getFeatureNum(void);
private:
    /* get extream point */
    int getExtremeFeature(void);

    /* delete features */
    void deleteFeatures(void);
    
    /* check and filter features */
    void checkExtreamFeature(void);

    /* gaussian eliminate arguement matrix */
    void gaussianElimination(float afAugmentedMatrix[3][4], float *pfResult);

    /* calculate orientations */
    void calcOrientations(DS_FEATURE *pstFeature, float *pfGradGauss);

    /* calculate key points */
    void calcKeypoints(void);

    /* calculate feature vectors */
    void calcFeatures(DS_FEATURE *pstFeature, float *pfGradGauss);
};

#endif
