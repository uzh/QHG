#ifndef __IDSAMPLER2_H__
#define __IDSAMPLER2_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"

#include "IDSample.h"

typedef struct locspec {
    const char *pLocFile;
    double dDistance;
    uint iNum;

    locspec(const char *pFile, double dDist=0, uint iN=0) 
        : pLocFile(pFile),dDistance(dDist),iNum(iN) {};
} locspec;

typedef struct locitem {
    double dLon;
    double dLat;
    double dDist;
    int iNum;
} locitem;

// map: location name => locitem
typedef std::map<std::string, locitem>               locdata;

// map: cell id => coord pair
typedef std::map<int, std::pair<double, double> >    coordmap;
// vector of string
typedef std::vector<std::string>                     stringvec; 




class IDSampler2 {

public:
    static IDSampler2 *createInstance(const char *pQDFGrid);
    ~IDSampler2();
    int init(const char *pQDFGrid);
    
    // the IDSample* returned by these functions must be cleaned up by the caller
    IDSample *getSamples(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, locdata &mLocData);
    IDSample *getAttributes(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, locdata  &mLocData, idset &sSelected);
    
    IDSample *getSamples(stringvec &vQDFPops, const char *pPopName, const locspec *pLocSpec, locdata &mLocData);
    IDSample *getAttributes(stringvec &vpQDFPops, const char *pPopName, const locspec *pLocSpec, locdata  &mLocData, idset &sSelected);



    // 
    IDSample *getSamples(const char *pQDFTime, const char *pPopName, const char *pLocFile, locdata &mLocData);
    IDSample *getAttributes(const char *pQDFTime, const char *pPopName, const char *pLocFile, locdata  &mLocData, idset &sSelected);
    
    IDSample *getSamples(stringvec &vQDFPops, const char *pPopName, const char *pLocFile, locdata &mLocData);
    IDSample *getAttributes(stringvec &vpQDFPops, const char *pPopName, const char *pLocFile, locdata  &mLocData, idset &sSelected);
    

    int getNumAgents() { return m_iNumAgents;};
    idtype *getIDs() { return m_pIDs;};


    
    // set of all selected ids
    void getFullIDSet(idset &sSelected); 
    void getFullIndexIDMap(indexids &mSelected); 
    // map:location => set of ids for all times
    void getLocationIDSet(locids &msSelected); 

    //    void getSampleInfo(sampleinfo &mmLocAgentData);
    
private:
    IDSampler2();
    int fillCoordMap(const char *pQDFGeoGrid);

    IDSample *getSamplesCore(const char *pQDFTime, const char *pPopName, locdata &mLocData);
    IDSample *getAttributesCore(const char *pQDFTime, const char *pPopName, locdata  &mLocData, idset &sSelected);

    int readArrays(const char *pQDFTime, const char *pPopName);
    int fillLocData(const locspec *pLocSpec, stringvec &vNames, locdata &mLocData);

    void deleteArrays();
    static int makeSelectionList(int iNumTotal, int iNumSelect, std::set<int> &sSelectedIndexes);

   
    coordmap m_mCoords;
    int m_iNumAgents;
    idtype *m_pIDs;
    int    *m_pCellIDs;
    int    *m_pGenders;
    float   m_fTimeStamp;

    IDSample *m_pCurSample;

 
};

#endif
