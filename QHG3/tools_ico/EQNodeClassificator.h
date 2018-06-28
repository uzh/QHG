#ifndef __EQNODECLASSIFICATOR_H__
#define __EQNODECLASSIFICATOR_H__

class EQsahedron;

typedef std::pair<int, int> intpair;
typedef std::map<intpair, intset> edgegroups;
typedef std::map<int, std::set<intpair> > pointtiles;

#define TYPE_VERT 0
#define TYPE_EDGE 1
#define TYPE_FACE 2
#define NUM_TYPES 3

class EQNodeClassificator {
public: 
    static EQNodeClassificator *createInstance(int iSubDivNodes, int iSubDivTiles);
    ~EQNodeClassificator();
    int applyToEQ(EQsahedron *pEQ);
    void distributeBorders();

    int  getFaceNodeTiles(int iFace, intset &sTiles);
    int  getEdgeNodeTiles(int iEdge, intset &sTiles);
    int  getVertNodeTiles(int iVert, intset &sTiles);

    int getNumTiles() {return ICOFACES*m_iNumTiles;};
    intset *getFaces() {return m_asBasicElements[TYPE_FACE];};
    intset *getEdges() {return m_asBasicElements[TYPE_EDGE];};
    intset *getVerts() {return m_asBasicElements[TYPE_VERT];};

    intset &getFinal(int iIndex) {return m_asFinal[iIndex];};

    int getSubDivNodes() { return m_iSubDivNodes;};
    int getSubDivTiles() { return m_iSubDivTiles;};
    void showBasicElements();
    void showAllClasses();
    void showFinal();
   
protected:
    EQNodeClassificator();
    int init(int iSubDivNodes, int iSubDivTiles);

    void classifySingleTile(int rT, int cT, bool bUpper, int f);
    void classifyNodes();

    void distributeElements();
    void makeFullEQ();

    void assignEdges();
    void assignVerts();
    void sanityCheck();

    int m_iSubDivNodes;
    int m_iSubDivTiles;
    int m_iNumTiles;
    int m_iRatio;
    intset **m_asBasicElements;
    intset **m_asAllElements;
    intset *m_asFinal;
    EQsahedron *m_pEQCur;
};


#endif
