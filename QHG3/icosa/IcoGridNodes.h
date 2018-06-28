#ifndef __ICOGRIDNODES_H__
#define __ICOGRIDNODES_H__

#include <map>
#include <string>
#include "icoutil.h"

typedef std::map<std::string, std::string> stringmap;

#define SURF_TYPE           "SURF_TYPE"
#define SURF_EQSAHEDRON     "IEQ"
#define SURF_ICOSAHEDRON    "ICO"
#define SURF_LATTICE        "LTC"
#define SURF_LTC_W          "W"
#define SURF_LTC_H          "H"
#define SURF_LTC_LINKS      "LINKS"
#define SURF_LTC_PERIODIC   "PERIODIC"
#define SURF_LTC_PROJ_TYPE  "PROJT"
#define SURF_LTC_PROJ_GRID  "PROJG"

#define SURF_IEQ_SUBDIVS    "SUBDIV"
#define SURF_IEQ_MINALT     "MINALT"

#define SURF_ICO_SUBLEVEL   "SUBLEVEL"

class IcoNode;

class IcoGridNodes {
public:
    IcoGridNodes();
    ~IcoGridNodes();
    

    int write(const char *pOutput, int iMaxLinks, bool bAddTilingInfo, stringmap &mAdditionalHeaderLines);
    int read(const char *pInput);
    int readHeader(FILE *fIn, int  *piMaxLinks, bool *pAddTilingInfo, stringmap *pAdditionalHeader, bool *pbVer);

    std::map<gridtype, IcoNode*> m_mNodes;
    int setTiledIDs( std::map<gridtype,gridtype> &mID2T);

    void setData(stringmap &smNew) { m_smData = smNew;};
    stringmap getData() { return m_smData; }; // no reference: want a copy
protected:
    int blockWrite(FILE *fOut, int iMaxLinks);
    int sequentialWrite(FILE *fOut, bool bAddTilingInfo);

    int blockRead(FILE *fIn, int iMaxLinks, bool bVer2);
    int sequentialRead(FILE *fIn, bool bAddTilingInfo, bool bVer2);

    stringmap m_smData;
};

#endif

