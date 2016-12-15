#ifndef __POPWRITER_H__
#define __POPWRITER_H__

#include <map>
#include <vector>
#include <string>
#include <hdf5.h>

class PopBase;

class PopWriter {
public:
    PopWriter(std::vector<PopBase *> vPops);
    ~PopWriter();
    
    int write(const char *pFilename, float fTime, char *pSub=NULL);
    int write(hid_t hFile, char *pSub=NULL);
    int openPopulationGroup() { return opencreatePopGroup();};
    void closePopulationGroup() { H5Gclose(m_hPopGroup);};
protected:
    std::map<PopBase *, hid_t> m_mDataTypes;
    hid_t    m_hFile;

    
    hid_t    m_hPopGroup;
    hid_t    m_hSpeciesGroup;
   
    int opencreatePopGroup();
    int opencreateSpeciesGroup(PopBase *pPB);
    
};


#endif
