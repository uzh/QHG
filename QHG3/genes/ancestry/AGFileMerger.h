#ifndef __AGFILEMERGER_H__
#define __AGFILEMERGER_H__

#include <stdio.h>
#include <vector>
#include <string>

#include "types.h"
#include "AncGraphBase.h"

class AncestorNode;
class RWAncGraph;
class BufWriter;

typedef struct aginfo {
    RWAncGraph *m_pAG;
    BufReader       *m_pBR;
    int              m_iNumToDo;
    aginfo(RWAncGraph *pAG, BufReader *pBR, int iNumToDo)
        : m_pAG(pAG),
          m_pBR(pBR),
          m_iNumToDo(iNumToDo) {};

} aginfo;
    
class AGFileMerger {
public:
    static AGFileMerger *createInstance(std::vector<std::string> &vAGFileNames, int iNumLoad);
    virtual ~AGFileMerger();

    int merge(const char *pOutputAG, int iBufSize, idset &sSelected, idset &sRoot);

protected:
    AGFileMerger();
    int init(std::vector<std::string> &vAGFileNames, int iNumLoad);
    aginfo *openFirst(const char *pAGFile);
    int loadNodes(aginfo *pagi);
    std::vector<aginfo *> m_vAGInfo;
    int m_iNumLoad;
    ulong m_iNumTotals;

};


#endif
