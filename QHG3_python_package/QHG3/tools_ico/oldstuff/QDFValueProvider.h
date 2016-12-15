#ifndef __QDFVALUEPROVIDER_H__
#define __QDFVALUEPROVIDER_H__

#include <map>
#include <string>

#include <hdf5.h>

#include "types.h"
#include "ValueProvider.h"

class QDFNodes;

struct qdfdata {
    std::string sFile;
    double *pdData;
    double dMinVal;
    double dMaxVal;
};

class QDFValueProvider : public ValueProvider {
public:
    static ValueProvider *createValueProvider(const char *pQDFFile);
    virtual ~QDFValueProvider();

    virtual int init(const char *pFile);
    virtual double getValue(gridtype lID);

    virtual int addFile(const char *pFile);
    virtual int setMode(const char *pMode);
    
    std::map<std::string, qdfdata *> &getDataSets() { return m_mDataSets;};
    //debug:
    void show();
protected:
    QDFValueProvider();
    int addFile(hid_t hFile);
    void prune();

    qdfdata *extractStandardArray(hid_t hGroup, const char *pDataSetName, int iIndex);
    qdfdata *extractCellDataArray(hid_t hGroup, const char *pDataSetName);
    qdfdata *extractPopArray(hid_t hGroup, const char *pPopName);
    template<typename T>
    qdfdata *extractArray(hid_t hGroup, const char *pDataSetName, int iIndex, T t);

    hid_t      m_hCurFile;
    int        m_iNumCells;
    QDFNodes  *m_pQN;
    std::map<std::string, qdfdata *> m_mDataSets;
    std::string m_sCurDataSet;
};

#endif
