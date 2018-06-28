#ifndef __IDSAMPLE_H__
#define __IDSAMPLE_H__

#include <map>
#include <set>
#include <vector>

#include "types.h"

typedef struct agdata {
    agdata() 
        : iID(-1),
          iMomID(-1),
          iDadID(-1),
          iGender(-1),
          iCellID(-1),
          dLon(-1),
          dLat(-1),
          iArrayPos(-1) {};
    agdata(const agdata *pAD)
        : iID(pAD->iID),
          iMomID(pAD->iMomID),
          iDadID(pAD->iDadID),
          iGender(pAD->iGender),
          iCellID(pAD->iCellID),
          dLon(pAD->dLon),
          dLat(pAD->dLat),
          iArrayPos(pAD->iArrayPos) {};
        
    idtype iID;
    idtype iMomID;
    idtype iDadID;
    int iGender;
    int iCellID;
    double dLon;
    double dLat;
    int iArrayPos;  // index in agent array (QDF)
} agdata;

// map: (location, time) => idset
typedef  std::map<std::pair<std::string, double>, idset>  tloc_ids;

// map: location name => id set
typedef std::map<std::string, idset>                      loc_ids;

// map: location name => arrpos list
typedef std::map<std::string, std::vector<int> >          loc_varrpos;

// map: time => agdata list)
typedef std::map<float, std::vector<agdata*> >            time_vagdata;

// map: location name => (map: time => agdata list)
typedef std::map<std::string, time_vagdata>               sampleinfo;

// map: location name => agdata list
typedef std::map<std::string, std::set<agdata *> >        loc_agd;

// map: array pos => ID
typedef std::map<int, idtype>                             arrpos_ids;

// map: ID => agdata
typedef std::map<idtype, agdata *>                        id_agd; 

class IDSample {
public:
    IDSample();
    virtual ~IDSample();
    
    void addAgentData(const char *pLocation, float fTime, agdata *pAD);

    void addAgentDataVec(const char *pLocation, float fTime, std::vector<agdata*> &vAGD);

    // set of all selected ids
    void getFullIDSet(idset &sSelected) const; 
    // map: array pos => ID
    void getFullIndexIDMap(arrpos_ids &mSelected) const; 
    // map:location => set of ids for all times
    void getLocationIDSet(loc_ids &msSelected) const; 
    // map:(time,location) => set of ids for all times
    void getTimeLocationIDSet(tloc_ids &msSelected) const; 
    // map:location => set of agdata for all times
    void getLocationADSet(loc_agd &msSelected) const; 
    // map: id => agdata
    void getIDADMap(id_agd &mIDAD) const;
    const sampleinfo &getSampleInfo() const { return m_mmLocTimeAg;};

    int write(const char *pOutputFile);    
    int read(const char *pInPutFile);
    void display();
private:
    sampleinfo m_mmLocTimeAg;
};



#endif

