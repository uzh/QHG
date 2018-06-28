#ifndef __STATUSWRITER_H__
#define __STATUSWRITER_H__

#include <hdf5.h>
#include <vector>
#include <string>


class SCellGrid;
class PopBase;
class PopWriter;
class GridWriter;
class GeoWriter;
class ClimateWriter;
class VegWriter;
class MoveStatWriter;
class NavWriter;

static const unsigned int WR_NONE =   0;
static const unsigned int WR_GRID =   1;
static const unsigned int WR_GEO  =   2;
static const unsigned int WR_CLI  =   4;
static const unsigned int WR_VEG  =   8;
static const unsigned int WR_STAT =  16;
static const unsigned int WR_NAV  =  32;
static const unsigned int WR_ALL  =  63;
static const unsigned int WR_POP  =  64;
static const unsigned int WR_POPENV  = 255;

class StatusWriter {
public:
    static StatusWriter *createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops);
    virtual ~StatusWriter();

    int write(const char*pFileName, float fTime, char *pSub, int iWhat, int iDumpMode=-1);
    int write(const char*pFileName, float fTime, int iWhat, int iDumpMode=-1);
    
    std::string &getError() {return m_sError;};
protected:
    StatusWriter();
    int init(SCellGrid *pCG, std::vector<PopBase *> vPops);


    hid_t m_hFile;
    PopWriter      *m_pPopW;
    GridWriter     *m_pGridW;
    GeoWriter      *m_pGeoW;
    ClimateWriter  *m_pCliW;
    VegWriter      *m_pVegW;
    MoveStatWriter *m_pMStatW;
    NavWriter      *m_pNavW;
    
    std::string m_sError;
};




#endif
