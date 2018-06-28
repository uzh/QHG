#ifndef __ALLELECOUNTER_h__
#define __ALLELECOUNTER_h__

#include <vector>
#include <algorithm>

#include "types.h"
#include "QDFUtils.h"

#define DEF_BITS_PER_NUC 2

#define POP_DS_GENOME          "Genome" 
#define POP_DS_AGENTS          "AgentDataSet" 
#define POP_ATTR_GENOME_SIZE   "Genetics_genome_size"
#define POP_ATTR_BITS_PER_NUC  "Genetics_bits_per_nuc"
#define POP_ATTR_MUT_RATE      "Genetics_mutation_rate" 
#define SEL_ALL      0
#define SEL_GENDER_F 1
#define SEL_GENDER_M 2


class AlleleCounter {
    
public:
    static AlleleCounter *createInstance(const char *pQDFPopFile, const char *pSpeciesName);

    int countAlleles(int iSelectionType);

    uint **getCounts()   { return m_ppCounts;};
    int getGenomeSize()  { return m_iGenomeSize;};
    int getNumNucs()     { return m_iNumNucs;};
    int getNumGenomes()  { return m_iNumSelected;};

    virtual ~AlleleCounter();

protected:
    
    AlleleCounter();
    int init(const char *pQDFFile, const char *pSpeciesName);

    int init_qdf(const char *pQDFPopFile, 
                 const char *pSpeciesName);

    
    int prepareArrays();

    
    int selectIndexes(int iSelectionType);
    int loadGenomes(int iNumPerBuf);
    
    int m_iGenomeSize;
    int m_iNumBlocks;
    uint m_iBitsPerNuc;
    int m_iNumNucs;
    int m_iNucsInBlock;
    int m_iNumGenomes;
    int m_iNumSelected;
    
    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    ulong **m_ppGenomes;
    uint  **m_ppCounts;
    std::vector<int> m_vSelectedIndexes;
    bool m_bVerbose;
    char *m_pPopName;
   
};

#endif
