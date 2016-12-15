#ifndef __ANCESTORBOX4_H__
#define __ANCESTORBOX4_H__

#include "types.h"
#include "LayerArrBuf.h"
#include "LBController.h"

#define ANCESTOR4_BLOCK_SIZE     4

#define ANCPOP_ATTR_LAYERSIZE   "LayerSize"
#define ANCESTOR_DATASET_NAME   "Ancestors"

class AncestorBox4 {

public:
    static AncestorBox4 *createInstance(uint iLayerSize, const char *pOutputFile = NULL);
    AncestorBox4();
    ~AncestorBox4();
    int init(uint iLayerSize, const char *pOutputFile = NULL);
    int setOutputFile(char *pOutputFile, bool bAppend);
      
    int copyBlock(int iIndex, idtype *pData, int iCount);
    void addBaby(idtype iBabyID, idtype iMotherID, idtype iFatherID, float fTime, gridtype iCellID);


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

