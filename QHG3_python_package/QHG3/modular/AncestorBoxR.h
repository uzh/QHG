#ifndef __ANCESTORBOXR_H__
#define __ANCESTORBOXR_H__


#include "LayerArrBuf.h"
#include "LBController.h"

#define ANCESTOR_BLOCK_SIZE     3

#define ANCPOP_ATTR_LAYERSIZE   "LayerSize"
#define ANCESTOR_DATASET_NAME   "Ancestors"

class AncestorBoxR {

public:
    static AncestorBoxR *createInstance(uint iLayerSize, const char *pOutputFile = NULL);
    AncestorBoxR();
    ~AncestorBoxR();
    int init(uint iLayerSize, const char *pOutputFile = NULL);
    int setOutputFile(char *pOutputFile, bool bAppend);
      
    int copyBlock(int iIndex, idtype *pData, int iCount);
    void addBaby(idtype iBabyID, idtype iMotherID, idtype iFatherID);


    int writeData();

    uint getLayerSize() { return m_iLayerSize;};

protected:
    uint m_iLayerSize;
    LBController **m_pLBC;
    LayerArrBuf<idtype> **m_aAncestors;
    FILE *m_fOut;
    int m_iNumThreads;
};


#endif

