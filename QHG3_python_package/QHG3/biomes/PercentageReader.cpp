#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "strutils.h"
#include "LineReader.h"
#include "PercentageReader.h"

//------------------------------------------------------------------------------
// constructor
//
PercentageReader::PercentageReader(const char *pPercentageFile, bool bRelative) 
    : m_bRelative(bRelative) {

    m_bOK = readPercentages(pPercentageFile);
}

PercentageReader::~PercentageReader() {
}

bool PercentageReader::isReady() {
    return m_bOK;
}

bool PercentageReader::hasSpecies(const char *pSpecies) {
    BIOME_PERC_MAPS::const_iterator it = m_mapBiomePercentages.find(pSpecies);
    return (it != m_mapBiomePercentages.end());
}

double PercentageReader::getPercentageRel(const char *pSpecies, uchar ucBiome) {
    return m_mapBiomePercentages[pSpecies][ucBiome] *
        m_mapBiomePercentages[TOTAL_PSEUDOSPECIES][ucBiome];
}

double PercentageReader::getPercentageAbs(const char *pSpecies, uchar ucBiome) {
    return m_mapBiomePercentages[pSpecies][ucBiome];
}

uchar PercentageReader::getBiome(char *pBiome) {
    uchar ucBiome = 0xff;
    
    for (ID_BIOME_MAP::iterator it = m_mapIDBiomes.begin(); (ucBiome == 0xff) && (it != m_mapIDBiomes.end()); it++) {
        if (it->second == pBiome) {
            ucBiome = it->first;
        }
    }
    return ucBiome;
}


void PercentageReader::display() {
    BIOME_PERC_MAPS::iterator it;
    for (it = m_mapBiomePercentages.begin(); it != m_mapBiomePercentages.end(); it++) {
        printf("%s (%d):\n", it->first.c_str(),getSpeciesIndex(it->first.c_str()) );
        BIOME_PERC_MAP::iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            printf("  %d %-9s -> %f\n", it2->first, m_mapIDBiomes[it2->first].c_str(), it2->second);
        }
    }
}

void PercentageReader::displaySpecies(bool bInLine) {
    BIOME_PERC_MAPS::const_iterator it;
    for (it = m_mapBiomePercentages.begin(); it != m_mapBiomePercentages.end(); it++) {
        if (it != m_mapBiomePercentages.begin()) {
            if (bInLine) {
                printf(", ");
            } else {
                printf("\n");
            }
        }
        printf("'%s'", it->first.c_str());
    }
    if (!bInLine) {
        printf("\n");
    }
}


//------------------------------------------------------------------------------
// readPercentages
//   read percentages from file
//
bool PercentageReader::readPercentages(const char *pPercentageFile) {
    bool bOK = false;

    LineReader *pLR = LineReader_std::createInstance(pPercentageFile, "rb");
    if (pLR != NULL) {
            
        char *pLine = pLR->getNextLine();
        printf("[PercentageReader::readPercentages] first line [%s]\n", pLine);
        bOK = splitHeader(pLine);
        if (bOK) {
            while (bOK && !pLR->isEoF()) {
                pLine = pLR->getNextLine();
                
                if (pLine != NULL) {
                    bOK = splitLine(pLine);
                }
            }
        } else {
            printf("couldn't split Header [%s]\n", pLine);
        }            
        
        delete pLR;
    } else {
        printf("Couldn't open percentage file %s\n", pPercentageFile);
    }
    
    return bOK;
}

int PercentageReader::getSpeciesIndex(const char *pSpecies) {
    int iIndex = -1;
    if ((pSpecies != NULL) && (*pSpecies != '\0')) {
        for (int k = 0; (iIndex < 0) && (k < 3); k++) {
            if (strcmp(pSpecies, m_vecSpecies[k].c_str()) == 0) {
                iIndex = k;
            }
        }
    }
    return iIndex;
}

bool PercentageReader::splitHeader(char *pHeader) {
    bool bOK = false;
    char *pCtx;
    char *p = strtok_r(pHeader, " ,;\t\n", &pCtx);
    if (p != NULL) {
        if (strcasecmp(p, "Biome")== 0) {
            p = strtok_r(NULL, " ,;\t\n", &pCtx);
            if (p != NULL) {
                if (strcasecmp(p, "ID")== 0) {
                    bOK = true;
                    p = strtok_r(NULL, " ,;\t\n", &pCtx);

                    while (bOK && (p != NULL)) {
                        m_vecSpecies.push_back(p);
                        p = strtok_r(NULL, " ,;\t\n", &pCtx);
                    }
                    m_vecSpecies.push_back(TOTAL_PSEUDOSPECIES);
                } else {
                    printf("Header must start with \"Biome\"\n");
                }
            } else {
                printf("Bad header [%s]\n", pHeader);
            }
        } else {
            printf("Header must start with \"Biome\"\n");
        }
    } else {
        printf("Bad header [%s]\n", pHeader);
    }
    return bOK;
}

 bool PercentageReader::splitLine(char *pLine) {
    bool bOK = false;
    VEC_DOUBLES vValues;
    char *pCtx;
    char *pEnd;
    char *pBiomeName = strtok_r(pLine, " ,;\t\n", &pCtx);
    if (pBiomeName != NULL) {
        // first should be the name
        // later: save it
        char *p = strtok_r(NULL, " ,;\t\n", &pCtx);
        if (p != NULL) {
            int iBiomeVal = strtol(trim(p), &pEnd, 10);
            if (*pEnd == '\0') {
                bOK = true;
                
                unsigned int iCount = 0;
                double dSum = 0;
                while (bOK && (p != NULL)) {
                    p = strtok_r(NULL, " ,;\t\n", &pCtx);
                    if (p != NULL) {
                        double dToVal = strtod(trim(p), &pEnd);
                        if (*pEnd == '\0') {
                            dSum += dToVal;
                            vValues.push_back(dToVal);
                            iCount++;
                        } else {
                            bOK = false;
                            printf("bad number format: %s\n", p);
                        }
                    }
                }
                
                if (iCount == m_vecSpecies.size()-1) {
                    for (unsigned int i = 0; i < iCount; i++) {
                        if (dSum > 0) {
                            m_mapBiomePercentages[m_vecSpecies[i]][iBiomeVal] = vValues[i] / dSum;
                        } else {
                            m_mapBiomePercentages[m_vecSpecies[i]][iBiomeVal] = 0;
                        }
                    }
                    m_vecBiomes.push_back(iBiomeVal);
                    m_mapIDBiomes[iBiomeVal] = pBiomeName;
                    m_mapBiomePercentages[TOTAL_PSEUDOSPECIES][iBiomeVal] = dSum;
                } else {
                    bOK = false;
                    printf("Bad line format: expected %zd values, found %d\n", m_vecSpecies.size(), iCount);
                }
            } else {
                bOK = false;
                printf("bad number format: %s\n", p);
            }  
        } else {
            bOK = false;
            printf("bad line format: [%s]\n", p);
        }
    } else {
        bOK = false;
        printf("bad line format: [%s]\n", pLine);
    }
    
    return bOK;
 }
