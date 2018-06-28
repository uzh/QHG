#ifndef __COALESCER_H__
#define __COALESCER_H__

#include <set>

#include "types.h"
#include "BufReader.h"

#define ANCTYPE_SIMPLE    3
#define ANCTYPE_SPACETIME 4

typedef struct {
  idtype   iID;
  idtype   iMomID;
  idtype   iDadID;
  float    fStep; ; // float fTime;
  gridtype iCellID;
} ancitem;

typedef ulong filepos;
typedef std::map<idtype, filepos> oracle;

typedef struct locts {
    float    fTime;
    gridtype iCellID;
    locts() {};
    locts(float fTime1, gridtype iCellID1)
        : fTime(fTime1),
          iCellID(iCellID1) {
    };

} locts;
typedef std::map<idtype, locts> idloctsmap;
typedef std::pair<idtype, locts>  timeentry;


typedef struct timecomp {
  bool operator() (const timeentry &te1, const timeentry &te2) {
      return te1.second.fTime > te2.second.fTime;}
} timecomp;

typedef std::vector<timeentry> timelist;
typedef std::set<timeentry, timecomp> timeset;



typedef struct {
    idtype iIDFirst;
    idset  sLocalIDs;
} blockset;

typedef std::pair<long, long>       ancrange;
typedef std::map<idtype, ancrange>  ancinfo;
typedef ancinfo::iterator           ancinfo_it;
typedef ancinfo::const_iterator     ancinfo_cit;


class Coalescer {
public:
    static Coalescer *createInstance(char *pAncFile, uint iAncType, int iBlockSize);
    static Coalescer *createInstance(char *pAncFile, int iBlockSize);
    ~Coalescer();
    

    int doCoalesce(idset &sOriginals, bool bParallel);
    timeset **getCoalescents() { return m_aasCoalescents;};
    const timeset *getCoalescents(idtype i) { return m_aasCoalescents[i];};
    const timeset getCoalescents(idtype i, idtype j) { return m_aasCoalescents[i][j];};
protected:
    Coalescer();
    int init(char *pAncFile, uint iAncType, int iBlockSize);
    void clearSets();
    int initSets(const idset &sOriginals);

    int createOracle();
    int findAncestorsForIDs(const idset &sIDs, idset &sAncestors);
    int findAncestorsForIDsPar(const idset &sIDs, idset &sAncestors);
    int getLocTimesForIDs(const std::vector<idtype> &vIDs, timelist &sTimeList);
    int calcIntersection(idset &s1, idset &s2, timeset &sCoal);

    uint    m_iNum;
    uint    m_iComparesToDo;
    idset  *m_asRTree;
    idset **m_aasCurIDs;
    idset  *m_asComparables;
   
    timeset **m_aasCoalescents;
    oracle    m_mOracle;

    int       m_iBlockSize;
    filepos   m_lFileSize;
    FILE     *m_fAnc;
    uint      m_iAncSize;
    idtype   *m_aAncBuf;

    int            m_iNumThreads;
    int            m_iNumBlocks;
    blockset      *m_asBlockIDs;
    FILE         **m_afIn;
    BufReader    **m_apBR;
    idset         *m_asAncestors;         
    int           *m_aiIndexes;
    int           *m_aiCounts;
};

#endif
