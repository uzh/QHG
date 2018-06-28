#ifndef __SIMPARAMS_H__
#define __SIMPARAMS_H__


#include "utils.h"
#include "WELL512.h"
#include "SPopulation.h"
 
class ParamReader;
class EventData;
class EventManager;
class LineReader;

class SCellGrid;
class Geography;
class Climate;  
class Vegetation;
class MoveStats;
class Navigation;
class PopulationFactory;
class PopBase;
class PopLooper;
class StatusWriter;
class IDGen;


#define CMD_SET_ITERS             "SET ITERS"
#define CMD_REMOVE_ACTION         "REMOVE ACTION"

#define DEF_LOG_FILE              "SimTest.log"
#define DEF_OUT_PREFIX            "output_"

// structure to hold event data
typedef struct eventdata {
    char *pEventType;
    char *pEventParams;
    char *pEventTimes;
    eventdata(char *pType, char *pParams, char *pTimes) {
        pEventType = new char[strlen(pType)+1];
        strcpy(pEventType,  pType);
        pEventParams = new char[strlen(pParams)+1];
        strcpy(pEventParams,  pParams);
        pEventTimes = new char[strlen(pTimes)+1];
        strcpy(pEventTimes,  pTimes);
    };
    ~eventdata() {
        delete[] pEventType;
        delete[] pEventParams;
        delete[] pEventTimes;
    };
} eventdata;


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
    int addEventTriggersOld(char *pEventDescription, float fCurTime=-1);

    static std::string addEventName(std::string &sEventString);

protected:
    int          m_iNumIters;
    int          m_iLayerSize;
    bool m_bHelp;
    hid_t m_hFile;
    LineReader *m_pLRGrid;

    ParamReader *m_pPR;       
    SCellGrid   *m_pCG;       
    Geography   *m_pGeo;      
    Climate     *m_pCli;      
    Vegetation  *m_pVeg;      
    Navigation  *m_pNav;      
    PopLooper   *m_pPopLooper;
    IDGen      **m_apIDG;

    StatusWriter *m_pSW;
    EventManager *m_pEM;
    
    bool          m_bHasMoveStats;
    Surface           *m_pSurface;
    PopulationFactory *m_pPopFac;

    bool m_bZipOutput;
    bool m_bResume;
    int  m_iStartStep;

    char m_sHelpTopic[MAX_PATH];
    char m_sOutputPrefix[MAX_PATH];
    char m_sOutputDir[MAX_PATH];
    char m_sConfigOut[MAX_PATH];

    uint32_t m_aulState[STATE_SIZE];
    uint     m_aiSeeds[NUM_SEEDS];
    std::vector<std::string> m_vDataDirs;
    std::vector<eventdata*>   m_vEvents;

    bool m_bCalcGeoAngles;

    float m_fStartTime;

    int    setHelp(bool bHelp);
    int    setHelpTopic(const char *pHelpTopic);


    int setGrid(const char *pFile);
    
    int setGeo(const char *pFile);

    int setClimate(const char *pFile);

    int setVeg(const char *pFile);

    int setNav(const char *pFile);

    int setPopList(char *pList);
    int setPops(const char *pFile);

    int setStats(const char *pFile);

    int createQDFTimeStamp(int iEventType, char *pEventParams, char *pTimeStamp);


    // -- event parsing --
    int processEvents(float fCurTime);
    int readEventsFromFile(char *pEventFile);
    int readEventFromString(char *pEventString);

    // -- input data --
    int setGrid(hid_t hFile, bool bUpdate = false);
    int setGridFromDefFile(const char *pDefFile);

    int setGeo(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setGeoFromDefFile(const char *pDefFile);

    int setClimate(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setClimateFromDefFile(const char *pDefFile);

    int setVeg(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setVegFromDefFile(const char *pDefFile);

    int setNav(hid_t hFile, bool bUpdate);

    int setStats(hid_t hFile);

    int setPops(hid_t hFile, const char *pPopName, bool bRequired);
    int setPopsFromPopFile(const char *pClsFile, const char *pDataFile);

    int setPopParams(const char *pParams);

    int setOutputPrefix(const char *pOutputPrefix);
    int setOutputDir(const char *pOutputDir);
    int setDataDirs(const char *pDataDirs);
    int setConfigOut(const char *pConfigOut);

    int setSeed(const char *pSeed);

    int readSeed(const char *pFileName, std::vector<uint32_t> &vulState);
    int setShuffles(char *pShuffles);


    bool exists(const char *pFile, char *pExists);
    int  createDir(const char *pDirName);

    int createSurface();
    int readAgentData(PopBase *pPop, const char *pAgentDataFile);


 
};


#endif

