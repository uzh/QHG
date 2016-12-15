#ifndef __SIMPARAMS_H__
#define __SIMPARAMS_H__


#include "utils.h"
#include "WELL512.h"
 
class ParamReader;
class EventData;
class EventManager;
class LineReader;

class SCellGrid;
class Geography;
class Climate;  
class Vegetation;
class MoveStats;
class PopulationFactory;
class PopBase;
class PopLooper;
class StatusWriter;
class IDGen;

#define EVENT_ID_NONE    -1
#define EVENT_ID_WRITE    1
#define EVENT_ID_GEO      2
#define EVENT_ID_CLIMATE  3
#define EVENT_ID_COMM     4

#define EVENT_TYPE_WRITE          "write"
#define EVENT_TYPE_GEO            "geo"
#define EVENT_TYPE_CLIMATE        "climate"
#define EVENT_TYPE_COMM           "comm"

#define EVENT_PARAM_WRITE_GRID    "grid"
#define EVENT_PARAM_WRITE_GEO     "geo"
#define EVENT_PARAM_WRITE_CLIMATE "climate"
#define EVENT_PARAM_WRITE_VEG     "veg"
#define EVENT_PARAM_WRITE_POP     "pop"
#define EVENT_PARAM_WRITE_STATS   "stats"
#define EVENT_PARAM_WRITE_ALL     "all"

#define EVENT_PARAM_GEO_SEA       "sea"
#define EVENT_PARAM_GEO_ALT       "alt"
#define EVENT_PARAM_GEO_QDF       "qdf"

#define DEF_LOG_FILE              "SimTest.log"
#define DEF_OUT_PREFIX            "output_"

class Surface;

class SimParams {
public:
    SimParams();
    virtual ~SimParams();

    int readOptions(int iArgC, char *apArgV[]);
    static void helpParams();
    static void showTopicHelp(const char *pTopic);

    const char *getHelpTopic() { return m_sHelpTopic;};

    int addEventTriggers(char *pEventDescription, float fCurTime=-1);

    static std::string addEventName(std::string &sEventString);

protected:
    int          m_iNumIters;
    int          m_iLayerSize;
    int          m_iShuffle;
    bool m_bHelp;
    hid_t m_hFile;
    LineReader *m_pLR;

    ParamReader *m_pPR;       
    SCellGrid   *m_pCG;       
    Geography   *m_pGeo;      
    Climate     *m_pCli;      
    Vegetation  *m_pVeg;      
    PopLooper   *m_pPopLooper;
    IDGen      **m_apIDG;

    StatusWriter *m_pSW;
    EventManager *m_pEM;
    
    bool          m_bHasMoveStats;
    Surface           *m_pSurface;
    PopulationFactory *m_pPopFac;

    char m_sHelpTopic[MAX_PATH];
    char m_sOutputPrefix[MAX_PATH];
    char m_sOutputDir[MAX_PATH];
    char m_sConfigOut[MAX_PATH];

    uint32_t m_aulState[STATE_SIZE];

    std::vector<std::string> m_vDataDirs;

    bool m_bCalcGeoAngles;

    float m_fStartTime;

    int    setHelp(bool bHelp);
    int    setHelpTopic(const char *pHelpTopic);


    int setGrid(const char *pFile);
    
    int setGeo(const char *pFile);

    int setClimate(const char *pFile);

    int setVeg(const char *pFile);

    int setPopList(char *pList);
    int setPops(const char *pFile);

    int setStats(const char *pFile);

    int createQDFTimeStamp(int iEventType, char *pEventParams, char *pTimeStamp);

    // -- input data --
    int setGrid(hid_t hFile, bool bUpdate = false);
    int setGridFromDefFile(const char *pDefFile);

    int setGeo(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setGeoFromDefFile(const char *pDefFile);

    int setClimate(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setClimateFromDefFile(const char *pDefFile);

    int setVeg(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setVegFromDefFile(const char *pDefFile);

    int setStats(hid_t hFile);

    int setPops(hid_t hFile, bool bRequired);
    int setPopsFromPopFile(const char *pClsFile, const char *pDataFile);

    int setPopParams(const char *pParams);

    int setOutputPrefix(const char *pOutputPrefix);
    int setOutputDir(const char *pOutputDir);
    int setDataDirs(const char *pDataDirs);
    int setConfigOut(const char *pConfigOut);
    int setSeed(const char *pSeed);

    bool exists(const char *pFile, char *pExists);
    int  createDir(const char *pDirName);

    int createSurface();
    int readAgentData(PopBase *pPop, const char *pAgentDataFile);


 
};


#endif

