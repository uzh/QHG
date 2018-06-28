#ifndef __POPWRITER_H__
#define __POPWRITER_H__

#include <map>
#include <vector>
#include <string>
#include <hdf5.h>

class PopBase;


typedef struct popwriteitem {
    hid_t m_hDataType;
    bool  m_bWriteAdditional;
    popwriteitem(hid_t hDataType, bool bWriteAdditional):
        m_hDataType(hDataType),
        m_bWriteAdditional(bWriteAdditional){};
    popwriteitem() :
        m_hDataType(H5P_DEFAULT),
        m_bWriteAdditional(true) {};
} popwriteitem;

typedef std::map<PopBase *, popwriteitem> popwriteitems; 

class PopWriter {
public:
    PopWriter(std::vector<PopBase *> vPops);
    ~PopWriter();
    
    int write(const char *pFilename, float fTime, char *pSub=NULL, int iDumpMode=-1);
    int write(hid_t hFile, char *pSub=NULL, int iDumpMode=-1); 
    int openPopulationGroup() { return opencreatePopGroup();};
    void closePopulationGroup() { H5Gclose(m_hPopGroup);};
protected:
    popwriteitems m_mDataTypes;
    hid_t    m_hFile;

    
    hid_t    m_hPopGroup;
    hid_t    m_hSpeciesGroup;
   
    int opencreatePopGroup();
    int opencreateSpeciesGroup(PopBase *pPB, int iDumpMode);
    
};


#endif
