#ifndef __GRIDFACTORY_H__
#define __GRIDFACTORY_H__

#include <string>
#include <vector>
#include "LineReader.h"

class GridFactory {
public:
    GridFactory(const char *sDefFile);
    ~GridFactory();

    bool isReady() { return (m_pLR != NULL);};
    int readDef();

    SCellGrid* getCellGrid() { return m_pCG; };
    Geography* getGeography() { return m_pGeo; };
    Climate* getClimate() { return m_pClimate; };
    Vegetation* getVeg() {return m_pVeg;};
protected:
    uint m_iNumCells;

    bool exists(const char *pFile, char *pExists);
    int setDataDirs(const char *pDataDirs);

    int collectLines(std::string *pLineList);

    int setGrid(const char *pLine);

    int createCells(IcoGridNodes *pIGN);
    int createCells(int iW, int iH, bool bPeriodic, bool bHex);

    int createCellsHexPeriodic(uint iW, uint iH);
    int createCellsHexNonPeriodic(uint iW, uint iH);

    int createCellsRectPeriodic(uint iW, uint iH);
    int createCellsRectNonPeriodic(uint iW, uint iH);

    int initializeGeography(IcoGridNodes *pIGN);
    int initializeGeography(int iW, int iH, bool bHex);


    int setAltitude(ValReader *pVRAlt);
    int setIce(ValReader *pVRIce);

    int setDensity(int iIndex, ValReader *pVRD);
    int setANPP(int iIndex, ValReader *pVRN);
    int setDensity(int iIndex, double dD);
    int setANPP(int iIndex, double dA);

    SCellGrid*  m_pCG;
    Geography*  m_pGeo;
    Climate*    m_pClimate;
    Vegetation* m_pVeg;

    std::vector<std::string> m_vDataDirs;
    LineReader* m_pLR;
};

#endif
