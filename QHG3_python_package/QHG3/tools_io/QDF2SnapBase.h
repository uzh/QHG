#ifndef __QDF2SNAPBASE_H__
#define __QDF2SNAPBASE_H__

#include <hdf5.h>

class QDF2SnapBase {
public:
    QDF2SnapBase();
    virtual ~QDF2SnapBase();

    int init(hid_t hFile, int iNumCells, const char *pGroupName, const char *pAttr);

    virtual int fillSnapData(char *pBuffer)=0;
protected:
    hid_t createSCellAgentType();
    int fillSimpleSnapDataDouble(char *pBuffer);
    int fillSimpleSnapDataInt(char *pBuffer);

    int   m_iNumCells;
    hid_t m_hGridGroup;
    hid_t m_hDataSetGrid;
    hid_t m_hDataSpaceGrid;

    hid_t m_hValueGroup;
    hid_t m_hDataSetValue;
    hid_t m_hDataSpaceValue; 

    const char *m_pAttr;

};

#endif

