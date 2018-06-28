#ifndef __BINPHENEFILE_H__
#define __BINPHENEFILE_H__

#include <string>
#include <vector>
#include <map>

#include "types.h"
#include "strutils.h"
#include "AnalysisUtils.h"

// id => phenome
typedef std::map<idtype, float*> id_phenomes;

class BinPheneFile {
public:
    static BinPheneFile *createInstance(const char *pFile);
    
    // read() returns number of genomes
    int read();

    int init(const char *pFile);
    virtual ~BinPheneFile();
    
    void showHeader();

    void getMaps(id_phenomes &mIDPhen, tnamed_ids &mvIDs, id_node &mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs);

    char  *getMagic() { return m_sMagic; };
    int    getPhenomeSize() { return m_iPhenomeSize; };
    int    getNumPhenomes() { return m_iNumPhenomes; };
    int    getNumLocs()     { return m_iNumLocs; };
    int    getFull()        { return m_bFull;};

    double getTime() { return m_dTime; }
    const id_phenomes &getIDPhen()     { return m_mIDPhen;};
    const tnamed_ids  &getvIDs()      { return m_mvIDs;};
    const id_node     &getIDNodes()   { return m_mIDNodes;};
    const id_locs     &getIDLocs()    { return m_mIdLocs;};
    const named_locs  &getNamedLocs() { return m_mNamedLocs;};

    //    int readStandardBin2(const char *pGenomeFile);
    //    int readStandardBin2a(const char *pGenomeFile); 

    FILE *getFileHandle() { return m_fIn; };
    char *getName() { return m_pCurName;};
private:
    BinPheneFile();
    int readHeader();
    int readLocHeader();
    int readPhenomeHeader();
    int readPhenome();
   


    char       *m_pCurName;
    char       *m_pSpecial;
    int         m_iSpecial;
    FILE       *m_fIn;
    char        m_sMagic[4];
    int         m_iPhenomeSize;
    int         m_iNumBlocks;
    id_phenomes m_mIDPhen;
    tnamed_ids  m_mvIDs;
    id_node     m_mIDNodes;
    id_locs     m_mIdLocs;
    named_locs  m_mNamedLocs;
    double      m_dTime;


    int        m_iNumPhenomes;
    int        m_iNumLocs;
    bool       m_bFull;
    
    idtype     m_iCurID;
    std::vector<idtype> m_vCurIDs;
    int        m_iNumSubPhenomes;
    
};

#endif
