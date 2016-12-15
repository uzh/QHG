#ifndef __POPBASE_H__
#define __POPBASE_H__

#include <hdf5.h>

#include <set>
#include "types.h"
#include "WELL512.h"
#include "SCellGrid.h"

class LineReader;
class PopBase {
public:
    virtual ~PopBase() {};
    
    virtual uint setPrioList() = 0;
    virtual uint getPrios(std::set<uint> &vPrios)=0;
    
    virtual int preLoop()=0;
    virtual int postLoop()=0;
    virtual int doActions(uint iPrio, float fTime)=0;
    virtual int initializeStep(float fTime)=0;
    virtual int finalizeStep()=0;

    virtual void registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo)=0;
    virtual void registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex=-1)=0;
    virtual void registerDeath(int iCellIndex, int iAgentIndex)=0;

    virtual void  showAgents()=0;
    virtual ulong getNumAgents(int iCellIndex)=0;
    virtual ulong getNumAgentsTotal()=0;
    virtual ulong getNumEvents(int iEventMask)=0;

    virtual int readSpeciesData(LineReader *pLR)=0;
    virtual spcid getSpeciesID()=0;
    virtual const char *getSpeciesName()=0;
    virtual size_t agentDataSize() const = 0; 
    virtual int addAgent(int iCellIndex, char *pData)=0;

    virtual size_t agentRealSizeQDF() const = 0; 
    virtual hid_t createAgentDataTypeQDF()=0;
    virtual int writeAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int writeSpeciesDataQDF(hid_t hSpeciesGroup)=0;
    virtual int readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType)=0;
    virtual int readSpeciesDataQDF(hid_t hSpeciesGroup)=0;
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup)=0;
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup)=0;

    virtual idtype getMaxLoadedID()=0;
    virtual idtype getUID()=0; 
    virtual void compactData()=0;
	virtual void checkLists()=0;

    virtual int  getNumCells()=0;

    virtual void randomize(int i)=0;

    virtual int   getFirstAgentIndex()=0;
    virtual int   getLastAgentIndex()=0;

    virtual int setParams(const char *pParams)=0;
    virtual int flushDeadSpace()=0;

    virtual uint     getAgentLifeState(int iAgentIndex)=0;
    virtual int      getAgentCellIndex(int iAgentIndex)=0;
    virtual idtype   getAgentID(int iAgentIndex)=0;
    virtual gridtype getAgentCellID(int iAgentIndex)=0;
    virtual float    getAgentBirthTime(int iAgentIndex)=0;
    virtual uchar    getAgentGender(int iAgentIndex)=0;
};

#endif
