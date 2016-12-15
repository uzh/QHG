#ifndef __RWANCGRAPH_H__
#define __RWANCGRAPH_H__

#include <set>
#include <map>
#include <vector>
#include "utils.h"
#include "AncGraphBase.h"

class LineReader;
class BufReader;
class BufWriter;

class AncestorNode;


//----------------------------------------------------------------------------
// RWAncGraph
//   
class RWAncGraph : public AncGraphBase {
public:
    RWAncGraph();
    virtual ~RWAncGraph();

    int loadBin(const char *pFileName);
    int saveBin(const char *pFileName);
    //    static int saveNode(BufWriter *pBW, AncestorNode *pAN);

    void addSelected(idtype iSelectedID);
    void setSelected(const idset &sSelected);

    AncestorNode *getAncestorNode(idtype iID);

    void showTree();
    void showAncestorInfo(bool bFull);
    
    int prune(bool bCut);

    int merge(RWAncGraph *pAG, bool bSets=true);
    const idset& getProgenitors(){ return m_sProgenitors;};
    const idset& getLeaves(){ return m_sLeaves;};
    void clearLeaves() { m_sLeaves.clear();}
    void clearProgenitors() { m_sProgenitors.clear();}

    void collectRootsLeaves();
    
private:
    idset m_sLeaves;
    idset m_sProgenitors;
    
    long   m_lListOffset;

    int saveBin(BufWriter *pBW, bool bWriteHeader, bool bWriteNodes, bool bWriteFooter);

    void markSurvivingLine(idtype iID);
    void deleteBranch(idtype iID);
  
};

#endif
