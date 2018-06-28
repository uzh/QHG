#ifndef __SPOPULATION_H__
#define __SPOPULATION_H__

#include <stdio.h>
#include <vector>

#include "types.h"
#include "WELL512.h"
#include "LineReader.h"
#include "icoutil.h"
#include "Prioritizer.h"
//#include "Prioritizer.cpp"
#include "IDGen.h"
#include "PopBase.h"

#include "LayerBuf.h"

#define SPOP_DT_LIFE_STATE "LifeState"
#define SPOP_DT_CELL_INDEX "CellIdx"
#define SPOP_DT_CELL_ID    "CellID"
#define SPOP_DT_AGENT_ID   "AgentID"
#define SPOP_DT_GENDER     "Gender"
#define SPOP_DT_BIRTH_TIME "BirthTime"
#define SPOP_DT_HOPS       "Hops"
#define SPOP_DT_PREV_DIST  "PrevDist"
#define SPOP_DT_PREV_LON   "PrevLon"
#define SPOP_DT_PREV_LAT   "PrevLat"

#define SPOP_AT_FUNCNAME   "FuncName"        
#define SPOP_AT_FUNCPRIO   "FuncPrio"

#define NUM_SEEDS           8

//class  L2List;
class LBController;
template<typename T> class Prioritizer;
class SCellGrid;
class PopFinder;

//struct __attribute__ (( aligned(64) )) Agent {
struct Agent {
    uint     m_iLifeState;
    int      m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;
};

/*
The basic Population class 
  - Species buffer (species specific data in environment)
  - Statistics (num born, num died etc)?

BirthQueue: mark agents as ready to deliver
DeathRow: mark agents as deathbound
I.e. add a field to agent structure
*/

#define MAX_FUNC_LEN 128
struct PrioData {
    char m_sFunction[MAX_FUNC_LEN];
    int  m_iPrioVal;
};

const int MAX_NAME = 32;

const uint LIFE_STATE_DEAD     =  0; 
const uint LIFE_STATE_ALIVE    =  1; 
const uint LIFE_STATE_FERTILE  =  5; // can be paired to a mate
const uint LIFE_STATE_MOVING   =  8;
const uint LIFE_STATE_SAILING  = 16;


const uint EVMSK_BIRTHS   = 1;
const uint EVMSK_MOVES    = 2;
const uint EVMSK_DEATHS   = 4;

const uint NOTIFICATION_EVENT = 1;
const uint NOTIFICATION_FLUSH = 2;



template<typename T>
class SPopulation : public PopBase {
public:
    // this will be the real constructor
    SPopulation(SCellGrid *pCG, PopFinder *m_pPopFinder, int iLayerSize, IDGen **apIDGen, uint32_t *aulState, uint *piSeeds);
    virtual ~SPopulation();

    // repeat the unimplemented methods from PopBase
    virtual uint setPrioList();
    virtual int  preLoop();
    virtual int  postLoop();
    virtual int doActions(uint iPrio, float fTime);
    // extend if derived class needs additional actions
    // before beginning step
    virtual int initializeStep(float fTime);
    virtual void initListIdx();

    // extend if derived class needs additional actions
    // at end of step
    virtual int finalizeStep();

    virtual void registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex=-1);
    virtual void registerDeath(int iCellIndex, int iAgentIndex);
    virtual void registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo);

    // event 
    //TODO: EventCreator    void registerForEvents(EventCreator *pEC) const;
    virtual int  updateEvent(int EventID, char *pData, float fT) { return 0;};
    virtual void flushEvents(float fT) {};

    virtual void compactData();

    virtual idtype getMaxLoadedID() { return m_iMaxID; };
    virtual idtype getUID(); 

    virtual void showAgents();
    virtual void showAgent(int iAgentIndex);
    
    virtual int readSpeciesData(LineReader *pLR);
    //    virtual int readSpeciesLine(char *pLine)=0;
    virtual spcid getSpeciesID() { return m_iSpeciesID;};
    virtual const char *getSpeciesName() { return m_sSpeciesName;};

    int addAgent(int iCellIndex, char *pData);
    int addAgentData(int iCellIndex, int iAgentIndex, char **ppData);
    template<typename T1>    int addAgentDataSingle(char **ppData, T1 *pAgentDataMember);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData) { return 0; };


    ulong getNumAgents(int iCellIndex);
    inline ulong getNumAgentsTotal() { return m_iTotal;};
    inline ulong getNumAgentsEffective() { return m_iTotal - m_iNumPrevDeaths;};
    virtual ulong getNumAgentsMax();

    void updateNumAgentsPerCell();
    void updateTotal();  
    virtual int checkLists();    

    virtual ulong getNumEvents(int iEventMask);

    void agentCheck();

    int   getFirstAgentIndex();
    int   getLastAgentIndex();

    virtual WELL512 **getWELL() { return m_apWELL;};
    virtual SCellGrid *getCG() { return m_pCG;};

    virtual void randomize(int i);
    virtual int flushDeadSpace();
    virtual int setParams(const char *pParams){ return 0;};

    // the array of agent structs
    LayerBuf<T> m_aAgents; 

    // auxiliary layerbuf for writing
    LayerBuf<T> m_aWriteCopy; 

    // agent-specific methods from PopBase
    virtual uint     getAgentLifeState(int iAgentIndex) {return m_aAgents[iAgentIndex].m_iLifeState;};
    virtual int      getAgentCellIndex(int iAgentIndex) {return m_aAgents[iAgentIndex].m_iCellIndex;};
    virtual idtype   getAgentID(int iAgentIndex) {return m_aAgents[iAgentIndex].m_ulID;};
    virtual gridtype getAgentCellID(int iAgentIndex) {return m_aAgents[iAgentIndex].m_ulCellID;};
    virtual float    getAgentBirthTime(int iAgentIndex) {return m_aAgents[iAgentIndex].m_fBirthTime;};
    virtual uchar    getAgentGender(int iAgentIndex) {return m_aAgents[iAgentIndex].m_iGender;};

    void showStates();
protected:
    int m_iNumCells;
    ulong m_iTotal;
    ulong m_iNumBirths;
    ulong m_iNumDeaths;
    ulong m_iNumMoves;
    double* m_dNumAgentsPerCell;

    float  m_fCurTime;

    uint32_t m_aulInitialState[STATE_SIZE];
 
    void prepareLists(int iAgentLayerSize, int iListLayerSize, uint32_t *aulState);
    // agentFactory functionality (must be extended by derived classes)
    int createNullAgent(int iCellIndex);   
    //@@ for recyleDeadSpace
    int createAgentAtIndex(int iAgentIndex, int iCellIndex);
    virtual int recycleDeadSpaceNew();

    virtual Agent *resetAgent(int iAgentIndex);

    // methods for agent moving
    // moved registerMove to public section
    //    virtual void registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo);
    virtual int moveAgent(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo);
    virtual int performMoves();

    // methods for agent births
    // moved registerBirth to public section
    //    virtual void registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex=-1);
    virtual int performBirths();
    void makeOffspring(int iCellIndex, int iMotherIndex, int iFatherIndex);
    //@@ for recyleDeadSpace
    virtual int performBirths(ulong iNumBirths, int *piBirthData);
    void makeOffspringAtIndex(int iAgentIndex, int iCellIndex, int iMotherIndex, int iFatherIndex);
    // virtual function for population-specific stuff for offspring creation
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather) { return 0; };    

    // methods for agent daeths
    // moved registerDeath to public section
    //    virtual void registerDeath(int iCellIndex, int iAgentIndex);
    virtual int performDeaths();
    //@@ for recyleDeadSpace
    virtual int performDeaths(ulong iNumDeaths, int *piDeathData);

    virtual size_t agentDataSize() const { return  sizeof(ulong)/*+sizeof(int)+3*sizeof(float)*/;};

    // HDF5 related
    hid_t createAgentDataTypeQDF();
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) { };
    int writeAgentDataQDF(hid_t dataspace_id, hid_t dataset_id, hid_t hAgentType);
    int writeAgentDataQDFSafe(hid_t dataspace_id, hid_t dataset_id, hid_t hAgentType);
    int writeSpeciesDataQDF(hid_t hSpeciesGroup);
    int readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType);
    int readSpeciesDataQDF(hid_t hSpeciesGroup);
    size_t agentRealSizeQDF() const {return sizeof(T);};
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int dumpAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType);
    virtual int restoreAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType);
    virtual int dumpSpeciesDataQDF(hid_t hSpeciesGroup, int iDumpMode);
    virtual int restoreSpeciesDataQDF(hid_t hSpeciesGroup);
    virtual int dumpController(hid_t hSpeciesGroup, int iDumpMode);
    virtual int restoreController(hid_t hSpeciesGroup);
    virtual int dumpDeadSpaces(hid_t hSpeciesGroup);
    virtual int restoreDeadSpaces(hid_t hSpeciesGroup);

    // for reading prio data from config file
    int readPrioInfo(char *pLine);
    std::map<std::string, int> m_mPrioInfo;
    int insertPrioDataAttribute(hid_t hSpeciesGroup);
    int extractPrioDataAttribute(hid_t hSpeciesGroup);
    virtual uint getPrios(std::set<uint> &vPrios) { return m_prio.getPrios(vPrios);};
    virtual int  removeAction(std::string name) {return m_prio.removeAction(name);};

    virtual int getNumCells() { return m_iNumCells; };


    void showControllerState(const char *pCaption);
    void showWELLStates(const char *pCaption, bool bNice);


    // class specific
    spcid m_iClassID;
    char  m_sClassName[MAX_NAME];
    // species specific
    spcid m_iSpeciesID;
    char  m_sSpeciesName[MAX_NAME];
    size  m_iSensingDistance;

    SCellGrid   *m_pCG;
    Prioritizer<T> m_prio;

    PopFinder *m_pPopFinder;
    // control for MemBlocks
    LBController *m_pAgentController;
    LBController *m_pWriteCopyController;

    std::vector<int>** m_vBirthList;
    std::vector<int>** m_vDeathList;
    std::vector<int>** m_vMoveList;

    std::vector<int> m_vMergedDeadList;

    WELL512 **m_apWELL;
    uint      m_aiSeeds[NUM_SEEDS];
    int m_iNumThreads;

    idtype  m_iMaxID;
    IDGen **m_apIDGen;

    
    int *m_pReuseB;
    int *m_pReuseD;
    ulong m_iMaxReuse;

    int *m_pNormalB;
    ulong m_iMaxNormalB;
    int *m_pNormalD;
    ulong m_iMaxNormalD;

    int *m_pPrevD;
    ulong m_iNumPrevDeaths;
    
    bool m_bRecycleDeadSpace;
//    double m_dRecycleDeadTime;
    double m_dDistScale;
    double (*m_fCalcDist)(double dX1, double  dY1, double dX2, double dY2, double dScale);
    
};


#endif
