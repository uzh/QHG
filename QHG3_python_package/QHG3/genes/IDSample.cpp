#include "types.h"
#include "utils.h"
#include "IDSampler2.h"


//----------------------------------------------------------------------------
// constructor
//
IDSample::IDSample() {
}

//----------------------------------------------------------------------------
// destructor
//
IDSample::~IDSample() {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                delete it_td->second[i];
            }
        }
    }

}

//----------------------------------------------------------------------------
// addAgentData
//  add a single agent data struct to the specified location and time
//
void IDSample::addAgentData(const char *pLocation, float fTimeStamp, agdata *pAD) {
    m_mmLocTimeAg[pLocation][fTimeStamp].push_back(pAD);
}



//----------------------------------------------------------------------------
// addAgentDataVec
//  add an entire vvector of agdata pointers to the specified location and time
void IDSample::addAgentDataVec(const char *pLocation, float fTimeStamp, std::vector<agdata*> &vAGD) {
    m_mmLocTimeAg[pLocation][fTimeStamp] = vAGD; 
}


//----------------------------------------------------------------------------
// getFullIDSet
//  fill sSelected with all ids
//
void IDSample::getFullIDSet(idset &sSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                sSelected.insert(it_td->second[i]->iID);
            }
        }
    }
}


//----------------------------------------------------------------------------
// getFullIndexIDMap
//  fill mSelected with Index=>ID pairs
// 
void IDSample::getFullIndexIDMap(indexids &mSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                mSelected[it_td->second[i]->iIndex] = it_td->second[i]->iID;
            }
        }
    }
}



//----------------------------------------------------------------------------
// getLocationIDSet
//  map: location name => set of IDs
//
void IDSample::getLocationIDSet(locids &msSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                msSelected[it_ltd->first].insert(it_td->second[i]->iID);
            }
        }
    }
    
}

//----------------------------------------------------------------------------
// getLocationIDSet
//  map: location name => set of agdata
//
void IDSample::getLocationADSet(locagd &msSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                msSelected[it_ltd->first].insert(it_td->second[i]);
            }
        }
    }
    
}

//----------------------------------------------------------------------------
// getIDADMap
//  map: ID => agdata
//
void IDSample::getIDADMap(idagd &mIDAD) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                mIDAD[it_td->second[i]->iID] = it_td->second[i];
            }
        }
    }
    
}
