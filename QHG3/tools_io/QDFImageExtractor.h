#ifndef __QDFIMAGEEXTRACTOR_H__
#define __QDFIMAGEEXTRACTOR_H__

#include <map>
#include <vector>
#include <string>

#include "QDFUtils.h"

class SurfaceGrid;
class AlphaComposer;
class LookUp;


#define ARR_CODE_NONE  -1
#define ARR_CODE_LON    1
#define ARR_CODE_LAT    2
#define ARR_CODE_ALT    3
#define ARR_CODE_ICE    4
#define ARR_CODE_WATER  5
#define ARR_CODE_COAST  6
#define ARR_CODE_TEMP   7
#define ARR_CODE_RAIN   8
#define ARR_CODE_NPP_B  9
#define ARR_CODE_NPP   10
#define ARR_CODE_DIST  11
#define ARR_CODE_TIME  12
#define ARR_CODE_POP   20

#define DS_TYPE_POP 14

// data set info: location, name, type
typedef struct ds_info{
    std::string sGroup;
    std::string sSubGroup;
    std::string sDataSet;
    int         iDataType;
    ds_info() 
        : sGroup(""), sSubGroup(""),sDataSet(""), iDataType(DS_TYPE_NONE) {};
    ds_info(std::string sGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(""), sDataSet(sDataSet1), iDataType(iDataType1) {};
    ds_info(std::string sGroup1, std::string sSubGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(sSubGroup1), sDataSet(sDataSet1), iDataType(iDataType1) {};
} ds_info;

typedef  std::pair<std::string, int>       arrind;
typedef  std::map<arrind, ds_info>         array_data;
typedef  std::map<arrind, LookUp *>        lookup_data; 
typedef  std::map<arrind, int>             which_data; 
typedef  std::vector<arrind>               data_order;
typedef  std::vector<std::string>          string_list;

// image properties: size, longitude shift
typedef struct img_prop {
    int iW;
    int iH;
    double dLonRoll;
    img_prop(int iW0, int iH0, double dLR0):iW(iW0),iH(iH0),dLonRoll(dLR0){};
} img_prop;

class QDFImageExtractor {
public:
    
    static QDFImageExtractor *createInstance(SurfaceGrid *pSG, 
                                             char        *sQDFGrid, 
                                             string_list &vQDFs, 
                                             char        *sArrayData, 
                                             img_prop    &ip, 
                                             bool         bVerbose);
    ~QDFImageExtractor();
    
    int extractAll(const char *sOutPat, const char *sCompOp, int iTime);
protected:
    QDFImageExtractor();
    int init(SurfaceGrid *pSG, 
             char        *sQDFGrid, 
             string_list &vQDFs, 
             char        *sArrayDatad, 
             img_prop    &ip, 
             bool         bVerbose);

    int checkArrayName(arrind &aiNameIndex);
    int splitArraySpec(char *sArrayData);
    int splitArrayColors(const char *sArrayData);
    int checkConsistent();
    bool checkGroup(hid_t hFile, const ds_info &rdInfo);
    double *extractData(const char *pQDF, const ds_info &pGroupDS);
    
    LookUp *createLookUp(char *pLUName, string_list &vParams);

    int LoopLayers(const char *sOutPat, const char *sCompOp);
    int addTimeLayer(int iTime);
    
    int writePNGFile(const char *pPat, const char *pReplace);
    bool extractColors(const char *pColorDesc, uchar *pR, uchar *pG, uchar *pB, uchar *pA);

    double **createDataMatrix(double *pData);

    void deleteMatrix(double **pData);
    void cleanUpLookUps();

    SurfaceGrid    *m_pSG;
    int    m_iW;
    int    m_iH;
    double m_dLonRoll;
    int             m_iNumLayers;

    string_list     m_vQDFs;
    array_data      m_mvArrayData;
    lookup_data     m_mvLookUpData;
    data_order      m_vOrder;
    which_data      m_mvWhich;
    int             m_iNumCells;

    AlphaComposer  *m_pAC;

    bool             m_bVerbose;
};

#endif
