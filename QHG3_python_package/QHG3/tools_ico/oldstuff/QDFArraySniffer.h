#ifndef __QDFARRAYSNIFFER_H__
#define __QDFARRAYSNIFFER_H__

#include <string>
#include <vector>

#include <hdf5.h>
#include "QDFUtils.h"

class QDFArraySniffer {

public:
    QDFArraySniffer(hid_t hFile, int iNumCells=0);
    int scan();
    int scanPopFile(hid_t hFile);
    std::vector<std::string> &getItems() { return m_vItems;};
private:
    int scanGroup(hid_t hRoot, const char *pName);
    int collectDataSets(hid_t loc_id, const char *pName);
    int scanPops(hid_t hLoc);
    
    hid_t m_hFile;
    std::vector<std::string> m_vItems;
    int m_iNumCells;
};



#endif
