#include <stdio.h>
#include <string.h>

#include "LookUp.h"
#include "BVLookUp.h"        // 2 params
#include "DiscreteLookUp.h"  // 1 param
#include "GeoLookUp.h"       // 3 params
#include "PopLookUp.h"       // 5 params
#include "UCharLookUp.h"     // 1 param
#include "VegLookUp.h"       // 3 params
#include "SunLookUp.h"       // 2 params
#include "RainbowLookUp.h"   // 2 params
#include "ThreshLookUp.h"    // 2 params
#include "DensityLookUp.h"   // 5 params
#include "BiomeLookUp.h"     // 1 param
#include "ZebraLookUp.h"     // 1 param
#include "SegmentLookUp.h"   // 3 param
#include "GenLookUp.h"       // 3 param

#include "LookUpFactory.h"
#include <stdarg.h>
#include <vector>


typedef struct {
    char sName[64];
    int           iType;
    unsigned int  iNumParams;
    std::vector<char*> *pvParamNames;
    std::vector<double> *pvParamDefs;
} LookUpData;


template <class T>
std::vector<T> *loadVector(int iNum, ... ) {
    va_list vl;
  
    std::vector<T> *pv = new std::vector<T>(); 
    va_start(vl, iNum);
    //    printf("have %d elements\n", iNum);
    for (int i = 0; i < iNum; i++) {
        pv->push_back(va_arg(vl, T));
    }

    va_end(vl);
    return pv;
}


LookUpData s_apLU[] = {
    {"UChar",   LOOKUP_UCHAR,   1, NULL, NULL},
    {"BinView", LOOKUP_BINVIEW, 2, NULL, NULL},
    {"Veg",     LOOKUP_VEG,     3, NULL, NULL},
    {"Pop",     LOOKUP_POP,     5, NULL, NULL},
    {"Geo",     LOOKUP_GEO,     3, NULL, NULL},
    {"Sun",     LOOKUP_SUN,     2, NULL, NULL},
    {"Rain",    LOOKUP_RAINBOW, 2, NULL, NULL},
    {"Thresh",  LOOKUP_THRESH,  1, NULL, NULL},
    {"Density", LOOKUP_DENSITY, 5, NULL, NULL},
    {"Biome",   LOOKUP_BIOME,   1, NULL, NULL},
    {"Zebra",   LOOKUP_ZEBRA,   1, NULL, NULL},
    {"Segment", LOOKUP_SEGMENT, 4, NULL, NULL},
    {"Gen",     LOOKUP_GEN,     2, NULL, NULL},
};
const int MIN_DIST_LEN = 3; // number of chars to distinguish names

LookUpFactory* LookUpFactory::s_pLUF = NULL;

LookUpFactory::LookUpFactory() {
    
   s_apLU[LOOKUP_UCHAR].pvParamNames   = loadVector<char*>(1, "NumCols");   
   s_apLU[LOOKUP_UCHAR].pvParamDefs    = loadVector<double>(1, 1.0);   
   s_apLU[LOOKUP_BINVIEW].pvParamNames = loadVector<char*>(2, "MinLevel", "MaxLevel");   
   s_apLU[LOOKUP_BINVIEW].pvParamDefs  = loadVector<double>(2, 0.0, 1.0);
   s_apLU[LOOKUP_VEG].pvParamNames     = loadVector<char*>(3, "MinLevel", "MaxLevel", "Type");
   s_apLU[LOOKUP_VEG].pvParamDefs      = loadVector<double>(3, 0.0, 1.0, 0.0);
   s_apLU[LOOKUP_POP].pvParamNames     = loadVector<char*>(5, "MaxLevel", "Red", "Green", "Blue", "Alpha");
   s_apLU[LOOKUP_POP].pvParamDefs      = loadVector<double>(5, 1.0, 1.0, 0.0, 0.0, 1.0);
   s_apLU[LOOKUP_GEO].pvParamNames     = loadVector<char*>(3, "MinLevel", "MaxLevel", "SeaLevel");
   s_apLU[LOOKUP_GEO].pvParamDefs      = loadVector<double>(3, -1.0,1.0,0.0);
   s_apLU[LOOKUP_SUN].pvParamNames     = loadVector<char*>(2, "MinLevel", "MaxLevel");   
   s_apLU[LOOKUP_SUN].pvParamDefs      = loadVector<double>(2, 0.0, 1.0);
   s_apLU[LOOKUP_RAINBOW].pvParamNames = loadVector<char*>(2, "MinLevel", "MaxLevel");   
   s_apLU[LOOKUP_RAINBOW].pvParamDefs  = loadVector<double>(2, 0.0, 1.0);
   s_apLU[LOOKUP_THRESH].pvParamNames  = loadVector<char*>(1, "Thresh");   
   s_apLU[LOOKUP_THRESH].pvParamDefs   = loadVector<double>(1, 0.5);
   s_apLU[LOOKUP_DENSITY].pvParamNames = loadVector<char*>(5, "MinLevel", "MaxLevel", "Red", "Green", "Blue");   
   s_apLU[LOOKUP_DENSITY].pvParamDefs  = loadVector<double>(5, 0.0, 1.0, 1.0, 0.0, 0.0);
   s_apLU[LOOKUP_BIOME].pvParamNames   = loadVector<char*>(1, "MaxLevel");   
   s_apLU[LOOKUP_BIOME].pvParamDefs    = loadVector<double>(1, 6.0);
   s_apLU[LOOKUP_ZEBRA].pvParamNames   = loadVector<char*>(1, "Width");   
   s_apLU[LOOKUP_ZEBRA].pvParamDefs    = loadVector<double>(1, 6.0);
   s_apLU[LOOKUP_SEGMENT].pvParamNames = loadVector<char*>(4, "MinLevel", "MaxLevel", "Width", "Subdiv");   
   s_apLU[LOOKUP_SEGMENT].pvParamDefs  = loadVector<double>(4, 0.0, 1.0, 0.1, 8.0);
   s_apLU[LOOKUP_GEN].pvParamNames     = loadVector<char*>(2, "MinLevel", "MaxLevel");   
   s_apLU[LOOKUP_GEN].pvParamDefs      = loadVector<double>(4, 0.0, 16777215.0);
        //   s_apLU[i].adParamDefs = s_adParamDefs[i];

   /*
   for (unsigned int i = 0; i < NUM_LOOKUPS; i++) {
       printf("*******\n");
       printf("%s:\n", s_apLU[i].sName);
       printf("  Num: %d\n", s_apLU[i].iNumParams);
       for (unsigned int j = 0; j < s_apLU[i].iNumParams; j++) {
           printf("  %s (%f)\n", (*s_apLU[i].pvParamNames)[j], (*s_apLU[i].pvParamDefs)[j]);
       }
   }
   */ 
};

LookUpFactory *LookUpFactory::instance() {
    if (s_pLUF == NULL) {
        s_pLUF = new LookUpFactory();
    }
    return s_pLUF;
}

void LookUpFactory::free() {
    if (s_pLUF != NULL) {
        delete s_pLUF;
    }
    s_pLUF = NULL;
}

LookUpFactory::~LookUpFactory() {
    delete s_apLU[LOOKUP_UCHAR].pvParamNames;
    delete s_apLU[LOOKUP_UCHAR].pvParamDefs;
    delete s_apLU[LOOKUP_BINVIEW].pvParamNames;
    delete s_apLU[LOOKUP_BINVIEW].pvParamDefs;
    delete s_apLU[LOOKUP_VEG].pvParamNames;
    delete s_apLU[LOOKUP_VEG].pvParamDefs;
    delete s_apLU[LOOKUP_POP].pvParamNames;
    delete s_apLU[LOOKUP_POP].pvParamDefs;
    delete s_apLU[LOOKUP_GEO].pvParamNames;
    delete s_apLU[LOOKUP_GEO].pvParamDefs;
    delete s_apLU[LOOKUP_SUN].pvParamNames;
    delete s_apLU[LOOKUP_SUN].pvParamDefs;
    delete s_apLU[LOOKUP_RAINBOW].pvParamNames;
    delete s_apLU[LOOKUP_RAINBOW].pvParamDefs;
    delete s_apLU[LOOKUP_THRESH].pvParamNames;
    delete s_apLU[LOOKUP_THRESH].pvParamDefs;
    delete s_apLU[LOOKUP_DENSITY].pvParamNames;
    delete s_apLU[LOOKUP_DENSITY].pvParamDefs;
    delete s_apLU[LOOKUP_BIOME].pvParamNames;
    delete s_apLU[LOOKUP_BIOME].pvParamDefs;
    delete s_apLU[LOOKUP_ZEBRA].pvParamNames;
    delete s_apLU[LOOKUP_ZEBRA].pvParamDefs;
    delete s_apLU[LOOKUP_SEGMENT].pvParamNames;
    delete s_apLU[LOOKUP_SEGMENT].pvParamDefs;
    delete s_apLU[LOOKUP_GEN].pvParamNames;
    delete s_apLU[LOOKUP_GEN].pvParamDefs;
}

int LookUpFactory::getNumLookUps() {
    return NUM_LOOKUPS;
}

char *LookUpFactory::getLookUpName(unsigned int iIndex) {
    char *p = NULL;
    if (iIndex < NUM_LOOKUPS) {
        p = s_apLU[iIndex].sName;
    }
    return p;
}

int LookUpFactory::getNumParams(unsigned int iLU) {
    int iNP = 0;
    if (iLU < NUM_LOOKUPS) {
        iNP = s_apLU[iLU].iNumParams;
    }
    return iNP;
}

char *LookUpFactory::getParamName(unsigned int iLU, unsigned int iParam) {
    char *p = NULL;
    if ((iLU < NUM_LOOKUPS) && (iParam < s_apLU[iLU].iNumParams)) {
        p = (*s_apLU[iLU].pvParamNames)[iParam];
    }
    return p;
}

double LookUpFactory::getParamDefault(unsigned int iLU, unsigned int iParam) {
    double d = 0;
    if ((iLU < NUM_LOOKUPS) && (iParam < s_apLU[iLU].iNumParams)) {
        d = (*s_apLU[iLU].pvParamDefs)[iParam];
    }
    return d;
}


LookUp *LookUpFactory::getLookUp(unsigned int iIndex, double *adParams, unsigned int iNumParams) {
    LookUp *pLU = NULL;
    printf("Looking for LU #%d\n", iIndex);
    if (iIndex < NUM_LOOKUPS) {
        switch (iIndex) {
        case LOOKUP_UCHAR:
            if (iNumParams == 0) {
                pLU = new UCharLookUp(1);
            } else {
                pLU = new UCharLookUp((int) adParams[0]);
            }
            break;
        case LOOKUP_BINVIEW: {
            double dMin=0;
            double dMax=1;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                }
            }
            pLU = new BVLookUp(dMin, dMax);
            break;
        }
        case LOOKUP_VEG: {
            double dMin=0;
            double dMax=1;
            int iType = 0;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                    if (iNumParams > 2) {
                        iType = (int) adParams[2];
                    }
                }
            }
            pLU = new VegLookUp(dMin, dMax, iType);
            break;
        }
        case LOOKUP_POP: {
            double dMax=1;
            double dR=1.0;
            double dG=0.0;
            double dB=0.0;
            double dA=1.0;

            if (iNumParams > 0) {
                dMax = adParams[0];
                if (iNumParams > 1) {
                    dR = adParams[1];
                    if (iNumParams > 2) {
                        dG = adParams[2];
                        if (iNumParams > 3) {
                            dB = adParams[3];
                            if (iNumParams > 4) {
                                dA = adParams[4];
                            }
                        }
                    }
                }
            }
            pLU = new PopLookUp(dMax, dR, dG, dB, dA);
            break;
        }
          
        case LOOKUP_GEO: {
            double dMin=-1;
            double dSea=0;
            double dMax=1;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                    if (iNumParams > 2) {
                        dSea = adParams[2];
                    }
                }
            }
            pLU = new GeoLookUp(dMin, dSea, dMax);
            break;
        }

        case LOOKUP_SUN: {
            double dMin=0;
            double dMax=1;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                }
            }
            pLU = new SunLookUp(dMin, dMax);
            break;
        }

        case LOOKUP_RAINBOW: {
            double dMin=0;
            double dMax=1;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                }
            }
            pLU = new RainbowLookUp(dMin, dMax);
            break;
        }

        case LOOKUP_THRESH: {
            double dThresh=0.5;
            if (iNumParams > 0) {
                dThresh = adParams[0];
            }
            pLU = new ThreshLookUp(dThresh);
            break;
        }

        case LOOKUP_DENSITY: {
            double dMin=0;
            double dMax=1;
            double dR=1.0;
            double dG=0.0;
            double dB=0.0;

            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                    if (iNumParams > 2) {
                        dR = adParams[2];
                        if (iNumParams > 3) {
                            dG = adParams[3];
                            if (iNumParams > 4) {
                                dB = adParams[4];
                            }
                        }
                    }
                }
            }
            pLU = new DensityLookUp(dMin,dMax, dR, dG, dB);        
            break;
        }
        case LOOKUP_BIOME:
            pLU = new BiomeLookUp();
            break;

        case LOOKUP_ZEBRA:
            if (iNumParams == 0) {
                pLU = new ZebraLookUp(10);
            } else {
                pLU = new ZebraLookUp((int) adParams[0]);
            }
            break;

        case LOOKUP_SEGMENT: {
            double dMin=0;
            double dMax=1;
            double dWidth = 0.2;
            double dSubDiv = 8.0;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                    if (iNumParams > 2) {
                        dWidth = adParams[2];
                        if (iNumParams > 3) {
                            dSubDiv = adParams[3];
                        }
                    } else {
                        dWidth = (dMax-dMin)/16;
                    }

                }
            }

            pLU = new SegmentLookUp(dMin, dMax, dWidth, dSubDiv);        
            break;
        }
        case LOOKUP_GEN: {
            double dMin=0;
            double dMax=1;
            if (iNumParams > 0) {
                dMin = adParams[0];
                if (iNumParams > 1) {
                    dMax = adParams[1];
                }
            }
            pLU = new GenLookUp(dMin, dMax);
            break;
        

        }
        
        }

    }
    return pLU;
}


int LookUpFactory::getLookUpType(const char *pLUName) {
    int iLUType = -1;
    for (int i = 0 ; (iLUType < 0) && (i <  getNumLookUps()); i++) {
        if (strncasecmp(pLUName, getLookUpName(i), MIN_DIST_LEN) == 0) {
            iLUType = i;
        }
    }

    return iLUType;
}

