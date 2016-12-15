#ifndef __ANCGRAPHBASE_H__
#define __ANCGRAPHBASE_H__

#include <map>
#include <set>

#include "types.h"

class BufWriter;
class BufReader;
class AncestorNode;

typedef std::map<idtype, AncestorNode *> ancnodelist;

typedef struct ANodeHeader {
    idtype iID;
    idtype iMomID;
    idtype iDadID;
    int    iGender;
    int    iNumChildren;
} ANodeHeader;

class AncGraphBase {
public:
    
    virtual ~AncGraphBase() {};
    
    AncestorNode *findAncestorNode(idtype iID);


    const ancnodelist& getMap(){ return m_mIndex;};
    ancnodelist& getModifiableMap(){ return m_mIndex;};
    const idset& getSelected(){ return m_sSelected;};
    const idset& getRoots(){ return m_sRoots;};
    virtual const idset& getProgenitors() = 0;

    static int saveNode(BufWriter *pBW, AncestorNode *pAN);
    AncestorNode *readNode(BufReader *pBR);
    int readNodeIf(BufReader *pBR, idtype iWantedID);
    int readNodeIf2(BufReader *pBR, idtype iWantedID, bool bShow);

    void clearSelected() {m_sSelected.clear();};
    void clearRoots() {m_sRoots.clear();};
    void clear(bool bDelete);

    static long getListOffset(const char *pFileName);
    static int readIntSetBin(BufReader *pBR, intset &sData);
    static int readIDSetBin(BufReader *pBR, idset &sData);
    static int writeIDSetBin(BufWriter *pBW, idset &sData);
    static int writeListOffset(const char *pFileName, long lListOffset);

protected:

    ancnodelist m_mIndex;
    idset      m_sSelected;
    idset      m_sRoots;
 
};


#endif

