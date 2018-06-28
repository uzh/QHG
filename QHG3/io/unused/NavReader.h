#ifndef __NAVREADER_H__
#define __NAVREADER_H__

#include <hdf5.h>

class Navigation;

class NavReader {
public:
    static NavReader *createNavReader(const char *pFileName);
    static NavReader *createNavReader(hid_t hFile);
    ~NavReader();

    int readAttributes(uint *piNumPorts, uint *piNumDests, uint *piNumDists, double *pdSampleDist);
    int readData(Navigation *pNav);
    
protected:
    NavReader();
    int init(const char *pFileName);
    int init(hid_t hFile);

    hid_t   m_hFile;
    hid_t   m_hNavGroup;
    uint    m_iNumPorts;
    uint    m_iNumDests;
    uint    m_iNumDists;
    double  m_dSampleDist;
};

#endif
