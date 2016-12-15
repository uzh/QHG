#ifndef __PERCENTAGE_READER_H__
#define __PERCENTAGE_READER_H__

#include "types.h"

const char TOTAL_PSEUDOSPECIES[] = "Total";

// Biome ID => biome name
typedef std::map<uchar, std::string> ID_BIOME_MAP;

// Biome ID => percentage
typedef std::map<uchar, double> BIOME_PERC_MAP;
// species name => map of all Biome ID => percentages for this species
typedef std::map<std::string, BIOME_PERC_MAP>  BIOME_PERC_MAPS;

class PercentageReader {
public:
    PercentageReader(const char *pPercentageFile, bool bRelative);
    virtual ~PercentageReader();

    bool isReady();
    bool hasSpecies(const char *pSpecies);
    
    double getPercentageAbs(const char *pSpecies, uchar ucBiome);
    double getPercentageRel(const char *pSpecies, uchar ucBiome);

    void display();
    void displaySpecies(bool bInLine);
    const BIOME_PERC_MAPS &getMap() {return m_mapBiomePercentages;}
    unsigned int  getNumBiomes() { return m_vecBiomes.size();};
    uchar getBiome(int iIndex) { return m_vecBiomes[iIndex];};
    uchar getBiome(char *pBiome);
    unsigned int  getNumSpecies() { return m_vecSpecies.size();};
    std::string getSpecies(int iIndex) { return m_vecSpecies[iIndex];};
    std::string getBiomeName(uchar ucBiome) { return m_mapIDBiomes[ucBiome];};
    int getSpeciesIndex(const char *pSpecies);
private:
    
    bool readPercentages(const char *pPercentageFile);
    bool splitHeader(char *pHeader);
    bool splitLine(char *pLine);
    
    bool        m_bOK;
    bool        m_bRelative;
    BIOME_PERC_MAPS  m_mapBiomePercentages;
    VEC_UCHARS   m_vecBiomes;
    VEC_STRINGS  m_vecSpecies;
    ID_BIOME_MAP m_mapIDBiomes;
};

#endif
