#ifndef __TEMPRAINBIOME_H__
#define __TEMPRAINBIOME_H__

#include <map>
#include <set>
//#include "FPointComp.h"
#include "types.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "ValueMapper.h"
#include "BStateVector.h"



typedef std::map<uchar, int> UIMAP;

static const char *SUFF_TRBQ  = "_trb.qmap";
//static const char *SUFF_TRBI  = "_trb.png";


typedef std::pair<int, int> loc;
typedef std::map<loc, BStateVector> BSBundle;
typedef std::map<uchar,int> biomestat;
typedef std::map<FPOINT, SET_UCHARS> TRBACC;

class TempRainBiome {

public:

    TempRainBiome();
    virtual ~TempRainBiome();
    bool init(char *pTemp, char *pRain);
    bool init(char *pTemp, char *pRain, char *pBiome);

    void process();
    void getExtremes();

    void makeTable(float fTempBinSize, float fRainBinSize, bool bPExtend, int iSmooth, char *pOutputFile, char *pTransFile, bool bVerbose);
    //    void makeLegend(char *pBiomeDoc, char *pOutputFile);

    char sPNGleg[MAX_PATH];


    void checkFiles(char *pNewTempFile, char *pNewRainFile, char *pOutputFile, const char *pSuff, float fDeltaTemp=0, float fScaleRain=1);
    void createBiomes(char *pTableFile, char *pOutputFile, const char *pSuff, float fDeltaTemp=0, float fScaleRain=1);

    bool  m_bVerbose;
private:
    uchar **createBiomes(float **pTData, float **pRData, float fDeltaTemp=0, float fScaleRain=1);
    void createBiomesCoord(float **pTData, float **pRData, float fDeltaTemp, float fScaleRain, uchar **aaucData, biomestat &mapBiomes);
    void createBiomesGrid(float **pTData, float **pRData, float fDeltaTemp, float fScaleRain, uchar **aaucData, biomestat &mapBiomes);


    int  collectMultiples();
    int  createBins(SET_UCHARS &su);
    void fillTableData(int iW, int iH);
    void uniquifyRing(int iW, int iH, int iD);
    void collectNeighborValuesRing(int iX, int iY, int iW, int iH, int iD, SET_UCHARS &B);
    void uniquify(int iW, int iH, int iD);
    void collectNeighborValues(int iX, int iY, int iW, int iH, int iD, UIMAP &B);
    int  markNeighbors(int iW, int iH);
    bool writeTRBTable(char *pOutput, int iW, int iH);
    bool writeBiomeFile(char *pOutput, uchar **aaucDataR);

    QMapReader<float>* checkClimateFile(char *pClimateFile, int *piW, int *piH);
    void calcMapIntersection(ValReader *pBR);
    void resetIntersection();
    void mapTRBTable(ValueMapper<uchar> *pVM);

    int createStateVectors();

    int m_iMaxTemp;
    int m_iMaxRain;

    float m_fMaxTemp;
    float m_fMinTemp;
    float m_fMaxRain;
    float m_fMinRain;

    float m_fMinBinTemp;
    float m_fMinBinRain;
    float m_fMaxBinTemp;
    float m_fMaxBinRain;

    float m_fTempBinSize;
    float m_fRainBinSize;

    QMapReader<uchar> *m_pQMRBiome;
    QMapReader<float> *m_pQMRTemp;
    QMapReader<float> *m_pQMRRain;

    //    TRBDATA     m_mapTRBData;
    // TRBACC: map :point(float,float)=>set(uchar)
    TRBACC m_mapTRBAcc;
    TRBACC m_mapMultiple;
    TRBACC m_mapBinned;
    uchar **m_aaucData;

    double m_dLonMin;
    double m_dLonMax;
    double m_dDLon;
    double m_dLatMin;
    double m_dLatMax;
    double m_dDLat;
    QMapHeader *m_pQMHIntersection;

    BSBundle m_bsb;
    int m_iMaxBiome;
    int m_iMaxBiomeR;
    
    bool m_bEqualGrids;
};



#endif

