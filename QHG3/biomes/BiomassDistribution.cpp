/******************************************************************************\
| Tes for the calculation of biomass distribution
|
\******************************************************************************/
#include <stdlib.h>
#include "TrinaryFunc.h"
#include "Const.h"
#include "Altitudinal.h"
#include "Latitudinal.h"
#include "LatAltitudinal.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapHeader.h"
#include "BiomassDist.h"
#include "strutils.h"
#include "LineReader.h"
#include "ParamReader.h"


const char *NON = "NONE";
const char *ALT = "ALTITUDINAL";
const char *LAT = "LATITUDINAL";
const char *TWO = "LATALT";

const double DEF_AMAP_LAT_MIN =  -89.833333333;
const double DEF_AMAP_LAT_MAX =   90.0;
const double DEF_AMAP_LON_MIN = -179.833333333;
const double DEF_AMAP_LON_MAX =  180.0;
const double DEF_AMAP_DLON    =  1.0/6.0;
const double DEF_AMAP_DLAT    = -1.0/6.0;

typedef std::map<std::string, double>  BIOME_VALUES;
/*
SPECIES MASSES
 for Savannah/Forest = 10% Parkland, 40% Grassland, 50% Bushland
   M_G =  3.55 ZJ
   M_B =  6.14 ZJ
   M_T = 20.31 ZJ

 for Savannah/Forest = 20% Parkland, 30% Grassland, 50% Bushland
   M_G =  3.63 ZJ
   M_B =  6.05 ZJ
   M_T = 20.32 ZJ
*/

//-----------------------------------------------------------------------------
// Usage
//
void Usage(char *pName) {
    printf("%s - calculate bio mass distribution for a plant\n", pName);
    printf("Usage:\n");
    printf("  %s, -b <BiomeBaseQMap> -p <PercentageFile> -f <FuncType>[:<Params>] \n", pName);
    printf("      -t <BiomeTargetFile> -o <outputfile>\n"); 
    printf("     ( -s <species-masses> | -g <biomeMassFile> )\n");
    printf("where\n");
    printf("  BiomeBaseQmap:   a QMap of biome values used to calculate densities\n");
    printf("  PercentageFile:  a textfile of lines of the form <biomevalue> <ratio>\n");
    printf("                   where ratio is the percentage of the biomass used by the plant in this biome\n");
    printf("  FuncType:        function describing the dependence of biomass on location\n");
    printf("  Params:          a comma-separated list of parameters for the func type\n");
    printf("                   currently the following functypes and params are available:\n");
    printf("                   - 'None'          (no params)\n");
    printf("                   - 'Altitudinal'   params: <altitudefile>,<maxAltitude>\n");
    printf("                   - 'Latitudinal'   (no params)\n");
    printf("                   - 'LatAlt'        params: <altitudefile>,<maxAltitude>\n");
    printf("  altitudefile     a 10' resolution binary short map covering the entire world\n");
    printf("  BiomeTargetQMap  biome file to be used to calculate the biomasses\n");
    printf("  species-masses   a list of the form <species1>:<mass1>,<species2>:<mass2>\n");
    printf("                   where massX is the total global biomass of the species X\n");
    printf("  biomeMassFile    name of a file containing masses for the biomes \n");
    printf("                   where massX is the total global biomass of the species X\n");
    printf("  OutputFile:      name of the output file\n");
}

//------------------------------------------------------------------------------
// selectFunc
//
TrinaryFunc *selectFunc(char *pType) {
    TrinaryFunc *pTF = NULL;
    double dDLon = DEF_AMAP_DLON;
    double dDataLonMin = DEF_AMAP_LON_MIN;
    double dDataLonMax = DEF_AMAP_LON_MAX;
    double dDLat = DEF_AMAP_DLAT;
    double dDataLatMin = DEF_AMAP_LAT_MIN;
    double dDataLatMax = DEF_AMAP_LAT_MAX;
    int    iHeaderSize = 0;

    char *pParams = strchr(pType, ':');
    if (pParams != NULL) {
        *pParams = '\0';
        ++pParams;
    }

    if ((strcasecmp(pType, ALT) == 0) || (strcasecmp(pType, TWO) == 0))  {
        if (pParams != NULL) {
            // expect filename,maxalt
            char *pAlt = strchr(pParams, ',');
            if (pAlt != NULL) {
                *pAlt = '\0';
                ++pAlt;
                char *pEnd;
                double dMaxAlt = strtod(trim(pAlt), &pEnd);
                if (*pEnd == '\0') {
                    

                    QMapHeader *pQMH = new QMapHeader();
                    int iResult = pQMH->readHeader(pParams);
                    if (iResult == 0) {
                        dDLon       = pQMH->m_dDLon;
                        dDataLonMin = pQMH->m_dDataLonMin;
                        dDataLonMax = pQMH->m_dDataLonMax;
                        dDLat       = pQMH->m_dDLat;
                        dDataLatMin = pQMH->m_dDataLatMin;
                        dDataLatMax = pQMH->m_dDataLatMax;
                        iHeaderSize = pQMH->getHeaderSize(pParams);
                    }
                    delete pQMH;
                    QMapReader<short int> *pQMR = 
                        new QMapReader<short int>(pParams, iHeaderSize,
                                                 dDLon, dDataLonMin, dDataLonMax,
                                                 dDLat, dDataLatMin, dDataLatMax, true);
                    bool bOK = pQMR->extractData();
                    if (bOK) {
                        if (strcasecmp(pType, ALT) == 0) {
                            pTF = new Altitudinal(pQMR, dMaxAlt, true);
                        } else {
                            pTF = new LatAltitudinal(pQMR, dMaxAlt, true);
                        }
                    } else {
                        printf("couldn't extract altitude data from %s\n", pParams);
                    }
                } else {
                    printf("Bad Number Format: %s\n", pAlt);
                }
            } else {
                printf("Parameters (AltitudeFile,MaxAltitude) required for type %s\n", ALT);
            }
        } else {
            printf("Parameters (AltitudeFile,MaxAltitude) required for type %s\n", ALT);
        }
    } else if (strcasecmp(pType, LAT) == 0)  {
        pTF = new Latitudinal();
    } else if (strcasecmp(pType, NON) == 0)  {
        pTF = new Const(1);
    }
    return pTF;
}

//------------------------------------------------------------------------------
// splitSpeciesMasses
//
int splitSpeciesMasses(char *pSpeciesMasses, VEC_STRINGS &vSpecies, MAP_STRINGDOUBLE &mTotalMasses) {
    printf("splitting [%s]\n", pSpeciesMasses);
    int iResult = 0;
    char *pCtx1;
    char *p = strtok_r(pSpeciesMasses, ",", &pCtx1);
    while ((iResult == 0) && (p != NULL)) {
        char *p2 = strchr(p, ':');
        if (p2 != NULL) {
            *p2 = '\0';
            p2++;
            vSpecies.push_back(p);
            char *pEnd;
            double dM = strtod(p2, &pEnd);
            if (*pEnd == '\0') {
                mTotalMasses[p] = dM;

                p = strtok_r(NULL, ",", &pCtx1);
            } else {
                printf("bad number format in species/masses specification:[%s]\n", p2);
                iResult = -2;
            }
        } else {
            printf("bad format in species/masses specification: missing \":\" [%s]\n", p);
            iResult = -1;
        }
    }
    printf("num: %zd\n", vSpecies.size());
    return iResult;
}

//------------------------------------------------------------------------------
// loadMasses
//
int loadMasses(const char *sBiomeMassFile,
               PercentageReader *pPR, 
               VEC_STRINGS &vSpecies, 
               MAP_STRINGDOUBLE &mTotalMasses) {
    int iResult = -1;
    BIOME_VALUES mapBiomeMasses;
    
    LineReader *pLR = LineReader_std::createInstance(sBiomeMassFile, "rt");
    if (pLR != NULL) {
        if (pLR->isReady()) {
            iResult = 0;
            while ((!pLR->isEoF()) && (iResult == 0)) {
                char *pLine = pLR->getNextLine();
                if (pLine != NULL ){
                    // expect Biome: value
                    char *pCtx;
                    char *p = strtok_r(pLine, ":", &pCtx);
                    if (p != NULL) {
                        char *p2 = strtok_r(NULL, ":", &pCtx);
                        char *pEnd;
                        double dVal = strtod(p2, &pEnd);
                        if (*pEnd == '\0') {
                            mapBiomeMasses[p] = dVal;
                            printf("Setting mass[%s] to %e\n", p, dVal); 
                        } else {
                            printf("Bad lineInvalid number in mass file: [%s]\n", p2);
                            iResult = -1;
                        }
                    
                    } else {
                        printf("Bad line in mass file: [%s]\n", pLine);
                        iResult = -1;
                    }
                }
            }
        } else {
            printf("Couldn't open [%s] for reading\n", sBiomeMassFile);
        }
        delete pLR;
    }


    if (iResult == 0) {
       
        // read biomemassfile: Biomename -> mass

        for (unsigned int i = 0; i < pPR->getNumSpecies(); i++) {
            std::string sSpecies = pPR->getSpecies(i);
            //            printf("Checking species [%s]\n", sSpecies.c_str());
            if (sSpecies != TOTAL_PSEUDOSPECIES) {
                vSpecies.push_back(sSpecies);
                double dSum = 0;
                for (unsigned int j = 0; (iResult == 0) && (j < pPR->getNumBiomes()); j++) {
                    uchar ucBiome = pPR->getBiome(j);
                    std::string sBiome = pPR->getBiomeName(ucBiome);
                    //                    printf("  Checking Biome [%s]\n", sBiome.c_str());
                    if (mapBiomeMasses.find(sBiome) != mapBiomeMasses.end()) {
                        //                        printf("    Adding %e*%f=%e\n",mapBiomeMasses[sBiome], pPR->getPercentageAbs(sSpecies.c_str(), ucBiome),mapBiomeMasses[sBiome]*pPR->getPercentageAbs(sSpecies.c_str(), ucBiome));  
                        dSum += mapBiomeMasses[sBiome]*pPR->getPercentageAbs(sSpecies.c_str(), ucBiome);
                    } else {
                        iResult = -1;
                    }
                }
                mTotalMasses[sSpecies] = dSum;
            }
        }
    }
    return iResult;
}

//------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sFuncDef[SHORT_INPUT];
    char sOutput[SHORT_INPUT];
    char sBiomeBaseFile[SHORT_INPUT];
    char sBiomeMassFile[SHORT_INPUT];
    char sBiomeTargetFile[SHORT_INPUT];
    char sPercentageFile[SHORT_INPUT];
    char sSpeciesMasses[SHORT_INPUT];
    char sType[SHORT_INPUT];
    *sFuncDef         = '\0';
    *sOutput          = '\0';
    *sBiomeBaseFile   = '\0';
    *sBiomeMassFile   = '\0';
    *sBiomeTargetFile = '\0';
    *sPercentageFile  = '\0';
    *sSpeciesMasses   = '\0';
    *sType            = '\0';

    int iType = TYPE_NONE;
    TrinaryFunc *pTF = NULL;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(8,
                               "-f:s!", sFuncDef,
                               "-o:s!", sOutput,  
                               "-b:s!", sBiomeBaseFile,
                               "-t:s!", sBiomeTargetFile,
                               "-p:s!", sPercentageFile,
                               "-s:s",  sSpeciesMasses,
                               "-g:s",  sBiomeMassFile,
                               "-T:s!", sType);
                    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (strncasecmp(sType, NAME_MASS, 4) == 0) {
                iType = TYPE_MASS;
            } else if (strncasecmp(sType, NAME_DENS, 4) == 0) {
                iType = TYPE_DENS;
            } else {
                printf("Unknown output type: [%s]\n", sType);
                iResult = -1;
            } 
            
            if (iResult == 0) {
                if ((*sSpeciesMasses != '\0') || (*sBiomeMassFile != '\0')) {
                    pTF = selectFunc(sFuncDef);
                    if (pTF != NULL) {
                        VEC_STRINGS vSpecies;
                        MAP_STRINGDOUBLE mTotalMasses;
                        PercentageReader *pPerc = new PercentageReader(sPercentageFile, true);
                        if (pPerc != NULL) {
                            if (*sSpeciesMasses != '\0') {
                                iResult = splitSpeciesMasses(sSpeciesMasses, vSpecies, mTotalMasses); 
                            } else {
                                iResult = loadMasses(sBiomeMassFile, pPerc, vSpecies, mTotalMasses);
                            }
                            if (iResult == 0) {
                                printf("BiomeBaseFile:   %s\n", sBiomeBaseFile);
                                printf("BiomeTargetFile: %s\n", sBiomeTargetFile);
                                printf("PercentageFile:  %s\n", sPercentageFile);
                                printf("OutputFile:      %s\n", sOutput);
                                printf("Type:            %s\n", sFuncDef);
                                printf("Species(Masses): ");
                                for (unsigned int i = 0; i < vSpecies.size(); i++) {
                                    if (i > 0) {
                                        printf(", ");
                                    }
                                    printf("%s(%e)", vSpecies[i].c_str(), mTotalMasses[vSpecies[i]]);
                                } 
                                printf("\n");
                            
                                printf("\n");
                            
                                BiomassDist *pBMD = new BiomassDist(sBiomeBaseFile);
                            
                                iResult = pBMD->calcGlobalDensity(mTotalMasses, vSpecies, pTF, sPercentageFile);
                                //    iResult = pBMD->calcDensity(mTotalMasses, vSpecies, pTF, sPercentageFile);
                            
                                //                printf("BaseMass: %f\n", dBaseMass);
                            
                                if (iResult == 0) {
                                    for (unsigned int i = 0; (iResult == 0) && (i < vSpecies.size()); i++) {
                                        const char *pSpecies = vSpecies[i].c_str();
                                        char sOut[SHORT_INPUT];
                                        sprintf(sOut, "%s_%s.qmap", sOutput, pSpecies);
                                        iResult =  pBMD->calcMass(sBiomeTargetFile, pSpecies, sOut, iType);
                                        if (iResult == 0) {
                                            printf("+++ success +++\n");
                                        } else {
                                            printf("Error during mass calc for %s\n", pSpecies);
                                        }
                                    }
                                } else {
                                    printf("Error during density calc\n");
                                }
                                /*
                                  clock_t t1 = clock();
                              
                
                                  QMapHeader *pQMH2 = new QMapHeader(QMAP_TYPE_DOUBLE,
                                  -29.75,60.01,0.5,
                                  25.25,65.01,0.5);
                                  double *pRow = new double[pQMH2->m_iWidth];
                                  FILE *fOut = fopen(sOutput, "wb");
                          
                                  if (fOut != NULL) {
                                  pQMH2->addHeader(fOut);
                                  double dLat0 = pQMH2->m_dDataLatMin;
                                  for (int iY = 0; iY < pQMH2->m_iHeight ; ++iY) {
                                  //???                        double dLat = pBMD->getQMapReader()->Y2Lat(iY);
                                  printf("Writing line %d lat %f", iY, dLat0); fflush(stdout);
                                  double dLon0 = pQMH2->m_dDataLonMin;
                                  for (int iX = 0; iX < pQMH2->m_iWidth; ++iX) {
                                  //???                            double dLon = pBMD->getQMapReader()->X2Lon(iX);
                                  pRow[iX] = pBMD->calcBiomass(dLon0, dLat0);
                                  //pRow[iX] = pBMD->getBiomass(iX, iY);
                                  dLon0 += pQMH2->m_dDLon;
                                  }
                                  dLat0 += pQMH2->m_dDLat;
                                  fwrite(pRow, sizeof(double), pQMH2->m_iWidth, fOut);
                                  }
                                  printf("\n");
                                  fclose(fOut);
                                  t1 = clock()-t1;
                                  printf("clocks: %.f\n", t1*1.0);
                                  } else {
                                  printf("Couldn't open %s\n", sOutput);
                                  }
                          
                                  delete[] pRow;
                                */
                                delete pBMD;
                            
                                delete pTF;
                            } else {
                                printf("Bad species/mass specification\n");
                            }
                        } else {
                            printf("Bad Percentage file [%s]\n", sPercentageFile);
                            iResult = -1;
                        }
                    } else {
                        printf("Bad function specification\n");
                    }
                } else {
                    printf("Either -s or -g must be specified\n");
                    iResult = -1;
                }
            } else {
                // bad type
            }
        } else {
            Usage(apArgV[0]);
        }
    } else {
        printf("Problem with setOptions\n");
    }

    delete pPR;
    return iResult;

}

