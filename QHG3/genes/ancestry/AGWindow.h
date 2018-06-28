#ifndef __AGWINDOW_H__
#define __AGWINDOW_H__

#include "AGOracle.h"

class AncestorNode;
class AncGraphBase;
class AGOracle;

class AGWindow {
public:
    static AGWindow *createInstance(const char *pAGFile, int iBlockSize);

    virtual ~AGWindow();

    int init(const char *pAGFile, int iBlockSize);

    int loadBlockFor(idtype iID);
    AncestorNode *getNode(idtype iID);

    // maybe some sort of iterator
    AncestorNode *getFirst();
    AncestorNode *getNext();
    const ancnodelist &getMap() {return m_pAG->getMap();};
    const idset &getRoots() { return m_pAG->getRoots();};
private:
    AGWindow();
    AncestorNode *getFirst(idtype iID);

    AncGraphBase *m_pAG;
    AGOracle     *m_pAGO;
    
    idtype   m_iCurMinID;
    idtype   m_iCurMaxID;

    ancnodelist::const_iterator m_itCur;
    ancinfo_cit m_itBlock;
};


#endif
