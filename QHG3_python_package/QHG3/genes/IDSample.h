#ifndef __IDSAMPLE_H__
#define __IDSAMPLE_H__

#include <map>
#include <set>
#include <vector>

typedef struct agdata {
    agdata() 
        : iID(-1),
          iMomID(-1),
          iDadID(-1),
          iGender(-1),
          iCellID(-1),
          dLon(-1),
          dLat(-1),
          iIndex(-1) {};
    agdata(const agdata *pAD)
        : iID(pAD->iID),
          iMomID(pAD->iMomID),
          iDadID(pAD->iDadID),
          iGender(pAD->iGender),
          iCellID(pAD->iCellID),
          dLon(pAD->dLon),
          dLat(pAD->dLat),
          iIndex(pAD->iIndex) {};
        
    idtype iID;
    idtype iMomID;
    idtype iDadID;
    int iGender;
    int iCellID;
    double dLon;
    double dLat;
    int iIndex;
} agdata;


// map: location name => id set
typedef std::map<std::string, idset>  locids;

// map: location name => index list
typedef std::map<std::string, std::vector<int> >  locindexes;
// map: time => agdata list)
typedef std::map<float, std::vector<agdata*> >    timeagdata;
// map: location name => (map: time => agdata list)
typedef std::map<std::string, timeagdata>         sampleinfo;

// map: location name => agdata list
typedef std::map<std::string, std::set<agdata *> > locagd;

// map: index => ID
typedef std::map<int, idtype>                     indexids;

// map: ID => agdata
typedef std::map<idtype, agdata *>                idagd; 

class IDSample {
public:
    IDSample();
    virtual ~IDSample();
    
    void addAgentData(const char *pLocation, float fTime, agdata *pAD);

    void addAgentDataVec(const char *pLocation, float fTime, std::vector<agdata*> &vAGD);

    // set of all selected ids
    void getFullIDSet(idset &sSelected) const; 
    // map: index => ID
    void getFullIndexIDMap(indexids &mSelected) const; 
    // map:location => set of ids for all times
    void getLocationIDSet(locids &msSelected) const; 
    // map:location => set of agdata for all times
    void getLocationADSet(locagd &msSelected) const; 
    // map: id => agdata
    void getIDADMap(idagd &mIDAD) const;
    const sampleinfo &getSampleInfo() const { return m_mmLocTimeAg;};

    
  
private:
    sampleinfo m_mmLocTimeAg;
};



#endif

