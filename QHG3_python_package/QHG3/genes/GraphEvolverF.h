#ifndef __GRAPHEVOLVERF_H__
#define __GRAPHEVOLVERF_H__

#include <map>

#include "types.h"
#include "LayerBuf.h"

#include "GraphEvolverBase.h"

class LBController;
class BinomialDist;
class AncestorGraph;
class AGOracle;

class GraphEvolverF : public GraphEvolverBase {
public:

    static GraphEvolverF *create(AncGraphBase *pAG, int iGenomeSize=-1, int iNumCrossovers=1, double dMutationRate=0);
    static GraphEvolverF *create(AncGraphBase *pAG, const char *pAGFile, int iOracleBlockSize, int iGenomeSize=-1, int iNumCrossovers=1, double dMutationRate=0);
    virtual ~GraphEvolverF();

    int calcGenomes(idset &sSelected);

    int buildGenome(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets);
    int buildGenomeSeq(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets);
    int buildGenomePar(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets);

    void setParLoad(bool bParLoad) { m_bParLoad = bParLoad;};
protected:
    virtual int init(int iGenomeSize, const char *pAGFile, int iOracleBlockSize);

private:
    GraphEvolverF(AncGraphBase *pAG, int iNumCrossovers=1, double dMutationRate=0);

    const ulong *calcGenome(idtype iID);

    int spreadMotherGenomes(const idset &sCurGen, idset &sNextList, idset &sSaveList);
    int updateLists(idset &sCurGen, const idset &sNextList, const idset &sSaveList);
    int dumpCompleted(const idset &sDumpList);


    AGOracle      *m_pAGO;

    void separateGenders(idset &sIn, idset &sOutM);
    void removeChildless(idset &sTargets, idset &sIn);
    int spreadMotherGenomesSeq(const idset &sCurGenM, 
                             idset &sDumpListM, idset &sDumpListD, 
                             idset &sNextListM);
    int updateListsSeq(idset &sCurGenM, 
                     const idset &sDumpListM, 
                     const idset &sNextListM);


    LBController            *m_pNodeController;
    LayerBuf<AncestorNode*>  m_aNodes; 

    int m_iNumThreads;
    int prepareNodesPar(const idset &sRoots);
    int spreadMotherGenomesPar(idset *asDumpListM, idset *asDumpListD, idset *asNextListM);
    int updateListsPar(const idset *asNextListM);
    idset *m_asChildlessDadIDs;
    idset *m_asChildlessMomListIndexes;
    idgenome *m_amNewGenomes;
    
    bool m_bParLoad;
  };


#endif

