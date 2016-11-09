#ifndef _DS_MATCH_H_
#define _DS_MATCH_H_

typedef struct matchedpoint
{
    unsigned int uiPosXFirst;
    unsigned int uiPosYFirst;
    unsigned int uiPosXSecond;
    unsigned int uiPosYSecond;
    bool         bIsSample;
}MATCHED_POINT;

class DS_MATCH
{
public:
    DS_MATCH(DS_SIFT *pstFirst, DS_SIFT *pstSecond);
    ~DS_MATCH(void);
private:
    DS_SIFT *pstFirst;
    DS_SIFT *pstSecond;
    std::vector <MATCHED_POINT *> vMatchedPoint; 
public:
    /* match image */
    int matchPictures(void);

    /* draw reault */
    int drawResult(char *pcFilePath);

    /* get matched point number */
    unsigned int getMatchedNum(void);

    /* ranasc remove bad matched points */
    void ransacFilter(void);
private:
    /* calculate the distance between two features */
    double calcDistance(float *pfFirst, float *pfSecond);

    /* delete all match points */
    void deleteMatchedPoint(void);

    /* draw lines */
    void drawLine(float *pfImage);

    /* random sample features */
    void ransacSample(std::vector <MATCHED_POINT *> *pvSamlple);

    /* calculate ransac matrix */
    int ransacCalcMatrix(std::vector <MATCHED_POINT *> *pvSamlple, float *pfMatrix);

    /* solve ransac matrix */
    int ransacSolveMatrix(float afMatrix[3][4],  float *pfResult);

    /* add points to set */
    void ransacAddtoModel(std::vector <MATCHED_POINT *> *pvSamlple, float *pfMatrix);

    /* ransac copy samples */
    void ransacCopySample(std::vector <MATCHED_POINT *> *pvDstSamlple, std::vector <MATCHED_POINT *> *pvSrcSamlple);
};

#endif
