#ifndef __IDSAMPLER2_H__
#define __IDSAMPLER2_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "WELL512.h"
#include "IDSample.h"
#include "AnalysisUtils.h"



class IDSampler2 {

public:
    static IDSampler2 *createInstance(const char *pQDFGrid, WELL512 *pWELL, bool bCartesian = false);
    ~IDSampler2();
    int init(const char *pQDFGrid, bool bCartesian = false);
    
    // the IDSample* returned by these functions must be cleaned up by the caller
    IDSample *getSamples(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, loc_data &mLocData, const locspec *pRefLocSpec, const char *pSampleOut = NULL);
    IDSample *getAttributes(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, loc_data  &mLocData, idset &sSelected);
    
    IDSample *getSamples(stringvec &vQDFPops, const char *pPopName, const locspec *pLocSpec, loc_data &mLocData, const locspec *pRefLocSpec, const char *pSampleOut = NULL);
    IDSample *getAttributes(stringvec &vpQDFPops, const char *pPopName, const locspec *pLocSpec, loc_data  &mLocData, idset &sSelected);



    // 
    IDSample *getSamples(const char *pQDFTime, const char *pPopName, const char *pLocFile, loc_data &mLocData, const char *pRefLocFile, const char *pSampleOut = NULL);
    IDSample *getAttributes(const char *pQDFTime, const char *pPopName, const char *pLocFile, loc_data  &mLocData, idset &sSelected);
    
    IDSample *getSamples(stringvec &vQDFPops, const char *pPopName, const char *pLocFile, loc_data &mLocData, const char *pRefLocFile, const char *pSampleOut = NULL);
    IDSample *getAttributes(stringvec &vpQDFPops, const char *pPopName, const char *pLocFile, loc_data  &mLocData, idset &sSelected);
    

    int getNumAgents() { return m_iNumAgents;};
    idtype *getIDs() { return m_pIDs;};


    
    // set of all selected ids
    void getFullIDSet(idset &sSelected); 
    void getFullIndexIDMap(arrpos_ids &mSelected); 
    // map:location => set of ids for all times
    void getLocationIDSet(loc_ids &msSelected); 

    // set of all ref ids
    void getRefIDSet(idset &sSelected); 
    void getRefIndexIDMap(arrpos_ids &mSelected); 
    // map:location => set of ids for all times
    void getRefLocationIDSet(loc_ids &msSelected); 

    std::string &getRefLocName() { return m_sRefLocName;};
    IDSample *getCurSample() { return m_pCurSample;};
    IDSample *getRefSample() { return m_pRefSample;};

private:
    IDSampler2(WELL512 *pWELL);
    int fillCoordMap(const char *pQDFGeoGrid);

    IDSample *getSamplesCore(const char *pPopName, loc_data &mLocData, const char *pRefLocName, const char *pSampleOut);
    int getCandidatesOld(loc_data &mLocData,   loc_varrpos &mvCandidates);
    int getCandidatesNew1(loc_data &mLocData,  loc_varrpos &mvCandidates);
    int getCandidatesNew2(loc_data &mLocData,  loc_varrpos &mvCandidates);
    IDSample *getAttributesCore(const char *pQDFTime, const char *pPopName, loc_data  &mLocData, idset &sSelected);

    int readArrays(const char *pQDFTime, const char *pPopName);

    void deleteArrays();
    /*static*/ int makeSelectionList(int iNumTotal, int iNumSelect, std::set<int> &sSelectedIndexes);

   
    arrpos_coords m_mCoords;
    int m_iNumAgents;
    idtype *m_pIDs;
    int    *m_pCellIDs;
    int    *m_pGenders;
    float   m_fTimeStamp;

    IDSample *m_pCurSample;
    IDSample *m_pRefSample;

    std::string m_sRefLocName;
    double m_dScale;
    double (*m_fCalcDist)(double dX1, double  dY1, double dX2, double dY2, double dScale);

    WELL512 *m_pWELL;
};

#endif
