#ifndef _DS_SIFT_H_
#define _DS_SIFT_H_

class DS_SIFT
{
public:
    DS_SIFT(DS_PGM *pstPic);
    virtual ~DS_SIFT(void);

    /* create scale space */
    int createScaleSpace(int iFirstOctave, int iLastOctave, int iFirstLevel, int iLastLevel, unsigned int uiOctaveResolution);

    /* create feature vector */
    int createFeatures(void);

    /* get all features */
    std::vector <DS_FEATURE *> *getAllFeatures(void);

    /* get feature number */
    unsigned int getFeatureNum(void);

    /* get image width */
    unsigned int getWidth(void);

    /* get image height */
    unsigned int getHeight(void);

    /* get mage data */
    float *getImageData(void);
private:
    DS_SCALESPACE *pstScaleSpace;
    DS_FEATURE_FILTER *pstFeatures;
    DS_PGM *pstImage;    
};

#endif
