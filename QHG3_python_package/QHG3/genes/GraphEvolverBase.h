#ifndef __GRAPHEVOLVERBASE_H__
#define __GRAPHEVOLVERBASE_H__

#include "types.h"
#include "GenomeProvider.h"

class BinomialDist;
class AncGraphBase;

typedef std::map<idtype, const ulong *> idgenome;

class GraphEvolverBase : public GenomeProvider {
public: 
    GraphEvolverBase(int iNumCrossovers, double dMutationRate);
    virtual ~GraphEvolverBase();
    int setGenomeSize(int iGenomeSize);
    virtual int getGenomeSize() { return m_iGenomeSize;};

    void setGenome(idtype iID, const ulong *pGenome) { m_mGenomes[iID] = pGenome;} ;
    virtual const ulong *getGenome(idtype iID);

protected:
    virtual int init(int iGenomeSize);

    ulong *createMix(ulong *pGenome1,  ulong *pGenome2);
    ulong *createMix(const ulong *pGenome1, const ulong *pGenome2);

    AncGraphBase *m_pAG;
    int m_iGenomeSize;
    int m_iNumBlocks;
    int m_iNumBits;
    int m_iNumCrossOvers;
    double m_dMutationRate;
    idgenome m_mGenomes;
    ulong **m_pTempGenome1;
    ulong **m_pTempGenome2;
    BinomialDist *m_pBDist;
    int m_iNumThreads;

    uint m_iLen;
    ulong **m_pAux;
};


#endif

