#ifndef __BINGENEFILE_H__
#define __BINGENEFILE_H__

#include <string>
#include <vector>
#include <map>

#include "types.h"
#include "strutils.h"
#include "AnalysisUtils.h"


// id => genome
typedef std::map<idtype, ulong*> id_genomes;


class BinGeneFile {
public:
    static BinGeneFile *createInstance(const char *pFile);
    
    // read() returns number of genomes
    int read();

    int init(const char *pFile);
    virtual ~BinGeneFile();
    
    void showHeader();

    void getMaps(id_genomes &mIDGen, tnamed_ids &mvIDs, id_node &mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs);

    char  *getMagic() { return m_sMagic; };
    int    getGenomeSize() { return m_iGenomeSize; };
    int    getNumGenomes() { return m_iNumGenomes; };
    int    getNumLocs()    { return m_iNumLocs; };
    int    getBitsPerNuc() { return m_iBitsPerNuc;};
    int    getFull()       { return m_bFull;};

    double getTime() { return m_dTime; }
    const id_genomes &getIDGen()     { return m_mIDGen;};
    const tnamed_ids &getvIDs()      { return m_mvIDs;};
    const id_node    &getIDNodes()   { return m_mIDNodes;};
    const id_locs    &getIDLocs()    { return m_mIdLocs;};
    const named_locs &getNamedLocs() { return m_mNamedLocs;};

    //    int readStandardBin2(const char *pGenomeFile);
    //    int readStandardBin2a(const char *pGenomeFile); 

    FILE *getFileHandle() { return m_fIn; };
    char *getName() { return m_pCurName;};
private:
    BinGeneFile();
    int readHeader();
    int readLocHeader();
    int readGenomeHeader();
    int readGenome();
   


    char      *m_pCurName;
    char      *m_pSpecial;
    int        m_iSpecial;
    FILE      *m_fIn;
    char       m_sMagic[4];
    int        m_iGenomeSize;
    int        m_iNumBlocks;
    id_genomes m_mIDGen;
    tnamed_ids m_mvIDs;
    id_node    m_mIDNodes;
    id_locs    m_mIdLocs;
    named_locs m_mNamedLocs;
    double     m_dTime;


    int        m_iNumGenomes;
    int        m_iNumLocs;
    int        m_iBitsPerNuc;
    bool       m_bFull;
    
    idtype     m_iCurID;
    std::vector<idtype> m_vCurIDs;
    int        m_iNumSubGenomes;
    bool       m_bExtended;
    
};

#endif
