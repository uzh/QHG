#ifndef __QDFNODES_H__
#define __QDFNODES_H__

#include <hdf5.h>

class QDFNodes {
public:
    static QDFNodes *createInstance(const char *pQDFFile);
    static QDFNodes *createInstance(hid_t hQDFFile);
    virtual ~QDFNodes();
    int getIndexForNode(int iNode);
    size_t getNumCells() { return m_mNode2Index.size();};
protected:
    QDFNodes();
    int init(const char *pQDFFile);
    int init(hid_t hQDFFile);
    int *m_pNodes;
    std::map<int, int> m_mNode2Index; 
};

#endif
