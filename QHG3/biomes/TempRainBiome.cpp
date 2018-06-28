#include <time.h>
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "TempRainBiome.h"
#include <algorithm>
#include <iterator>
#include "ValueMapper.h"
#include "IdentityMapper.h"
#include "IdentityMapper.cpp"
#include "SimpleMapper.h"
#include "SimpleMapper.cpp"
#include "BStateVector.h"


const uchar VAL_OCEAN = 0x00;
const uchar VAL_MULTI = 0xff;
const uchar VAL_UNDEF = 0xfe;

const int aiNeighborX[] = {-1, -1, -1, 0, 1, 1,  1,  0};
const int aiNeighborY[] = {-1,  0,  1, 1, 1, 0, -1, -1};
const int NUM_NEIGH = sizeof(aiNeighborX)/sizeof(int);

const int weights[] = {0, 10, 5, 5, 1, 20, 10, 1, 20, 10, 1, 1, 1, 1, 1};

//------------------------------------------------------------------------------
// displaySet
//
void displaySet(SET_UCHARS s) {
    printf("{");
    SET_UCHARS::iterator ot;
    for (ot = s.begin(); ot != s.end(); ++ot) {
        if (ot != s.begin()) {
            printf(", ");
        }
        printf("%d", *ot);
    }
    printf("}");
}

//------------------------------------------------------------------------------
// displaySet
//
void displaySet(UIMAP s) {
    printf("{");
    UIMAP::iterator ot;
    for (ot = s.begin(); ot != s.end(); ++ot) {
        if (ot != s.begin()) {
            printf(", ");
        }
        printf("(%d,%d)", ot->first, ot->second);
    }
    printf("}");
}

//------------------------------------------------------------------------------
// display
//
void display(TRBACC aa) {
    TRBACC::iterator it;
    for (it = aa.begin(); it != aa.end(); ++it) {
        printf("(%f %f)->", it->first.first, it->first.second);
        displaySet(it->second);
        printf("\n");
    }
}

//------------------------------------------------------------------------------
// constructor
//
TempRainBiome::TempRainBiome() 
    : m_bVerbose(false),
      m_pQMRBiome(NULL),
      m_pQMRTemp(NULL),
      m_pQMRRain(NULL),
      m_aaucData(NULL), 
      m_dLonMin(dNegInf),
      m_dLonMax(dPosInf),
      m_dDLon(dPosInf),
      m_dLatMin(dNegInf),
      m_dLatMax(dPosInf),
      m_dDLat(dPosInf),
      m_pQMHIntersection(NULL),
      m_iMaxBiome(0),
      m_bEqualGrids(false) {
}

//------------------------------------------------------------------------------
// destructor
//
TempRainBiome::~TempRainBiome() {
    if (m_pQMRBiome != NULL) {
        delete m_pQMRBiome;
    }
    if (m_pQMRTemp != NULL) {
        delete m_pQMRTemp;
    }
    if (m_pQMRRain != NULL) {
        delete m_pQMRRain;
    }
    if (m_pQMHIntersection != NULL) {
        delete m_pQMHIntersection;
    }

    if (m_aaucData != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxRain, m_aaucData);
    }

}

//------------------------------------------------------------------------------
// resetIntersection
//
void TempRainBiome::resetIntersection() {
    m_bEqualGrids = false;


    if (m_pQMHIntersection != NULL) {
        delete m_pQMHIntersection;
        m_pQMHIntersection = NULL;
    }
    m_dLonMin = dNegInf;
    m_dLonMax = dPosInf;
    m_dDLon   = dPosInf;
    m_dLatMin = dNegInf;
    m_dLatMax = dPosInf;
    m_dDLat   = dPosInf;
}

//------------------------------------------------------------------------------
// calcMapIntersection
//
void TempRainBiome::calcMapIntersection(ValReader *pVR) {
    double dVal = pVR->getLonMin();
    if (dVal > m_dLonMin) {
        m_dLonMin = dVal;
    }
    dVal = pVR->getLonMax();
    if (dVal < m_dLonMax) {
        m_dLonMax = dVal;
    }
    dVal = pVR->getDLon();
    if (dVal < m_dDLon) {
        m_dDLon = dVal;
    }
    dVal = pVR->getLatMin();
    if (dVal > m_dLatMin) {
        m_dLatMin = dVal;
    }
    dVal = pVR->getLatMax();
    if (dVal < m_dLatMax) {
        m_dLatMax = dVal;
    }
    dVal = pVR->getDLat();
    if (dVal < m_dDLat) {
        m_dDLat = dVal;
    }

}

//------------------------------------------------------------------------------
// init
//   read the data files and extract the data
//   The data files must all be QMap files
//   this method will be called if a trb-table must be created
//
//bool TempRainBiome::init(char *pTemp, char *pRain, char *pBiome, 
//                         float fRLonMin, float fRLonMax, float fRLatMin, float fRLatMax) {
bool TempRainBiome::init(char *pTemp, char *pRain, char *pBiome) {
    bool bOK = true;
    if (bOK) {
        printf("[init] reading biome file %s\n", pBiome);
        int iType = QMAP_TYPE_NONE;
        m_pQMRBiome = dynamic_cast<QMapReader<uchar>*>(QMapUtils::createValReader(pBiome, false, &iType));
        if (m_pQMRBiome != NULL) {
            if (iType == QMAP_TYPE_UCHAR) {
                bOK = true;
            } else {
                printf("Biome map should be of type %s but is %s(%d)\n", QMapHeader::getTypeName(QMAP_TYPE_UCHAR), QMapHeader::getTypeName(iType), iType);
                bOK = false;
            }
        } else {
            printf("COuldn't open biome map [%s]\n", pBiome);
            bOK = false;
        }
    }

    if (bOK) {
        //        bOK = init(pTemp, pRain,  fRLonMin, fRLonMax, fRLatMin, fRLatMax);
        bOK = init(pTemp, pRain);
    } else {
        if (m_pQMRBiome != NULL) {
            delete m_pQMRBiome;
            m_pQMRBiome = NULL;
        }
    }

    return bOK;
}

//------------------------------------------------------------------------------
// init
//   read the data files and extract the data
//   The data files must all be QMap files
//   This method will be called if biomes have to be created for a given TRB table
//
//bool TempRainBiome::init(char *pTemp, char *pRain,
//                         float fRLonMin, float fRLonMax, float fRLatMin, float fRLatMax) {
bool TempRainBiome::init(char *pTemp, char *pRain) {
    bool bOK = true;
    int iWB;
    int iHB;
    int iWT;
    int iHT;
    int iWR;
    int iHR;
  
    /*
    m_dLonMin = fRLonMin;
    m_dLonMax = fRLonMax;
    m_dLatMin = fRLatMin;
    m_dLatMax = fRLatMax;
    */

    resetIntersection();

    if (m_pQMRBiome != NULL) {
        iWB = m_pQMRBiome->getNRLon();
        iHB = m_pQMRBiome->getNRLat();
        calcMapIntersection(m_pQMRBiome);
        printf("[init] Biome: %dx%d\n", iWB, iHB);
    }

    m_pQMRTemp = checkClimateFile(pTemp, &iWT, &iHT);
    if (m_pQMRTemp != NULL) {
        calcMapIntersection(m_pQMRTemp);
    } else {
        bOK = false;
    }


    m_pQMRRain = checkClimateFile(pRain, &iWR, &iHR);
    if (m_pQMRRain != NULL) {
        calcMapIntersection(m_pQMRRain);
    } else {
        bOK = false;
    }


    if (bOK) {
        // check if temp and rain are equally formatted
        if ((iWT == iWR) && (iHT == iHR)) {
            if ((m_pQMRTemp->getDLon()   == m_pQMRRain->getDLon()) &&
                (m_pQMRTemp->getDLat()   == m_pQMRRain->getDLat()) &&
                (m_pQMRTemp->getLonMin() == m_pQMRRain->getLonMin()) &&
                (m_pQMRTemp->getLatMin() == m_pQMRRain->getLatMin())) {
                m_bEqualGrids = true;
                printf("Equal Lon: [%f,%f]:%f\n", m_dLonMin, m_dLonMax, m_dDLon);
                printf("Equal Lat: [%f,%f]:%f\n", m_dLatMin, m_dLatMax, m_dDLat);
            }

        }

        //   if ((iWB == iWT) && (iWB == iWR) && (iHB == iHT) && (iHB == iHR)) {

        printf("Intersection1:\n");
        printf("Lon: [%f,%f]:%f\n", m_dLonMin, m_dLonMax, m_dDLon);
        printf("Lat: [%f,%f]:%f\n", m_dLatMin, m_dLatMax, m_dDLat);
        
        m_pQMHIntersection = new QMapHeader(QMAP_TYPE_UCHAR,  
                                            m_dLonMin, m_dLonMax, m_dDLon,
                                            m_dLatMin, m_dLatMax, m_dDLat);
        
        


        bOK = true;

        
        /*
          } else {
          printf("Size mismatch:\n");
          printf("  Biome: %dx%d\n", iWB, iHB);
          printf("  Temp:  %dx%d\n", iWT, iHT);
          printf("  Rain:  %dx%d\n", iWR, iHR);
          bOK = false;
          }
        */
    }

    return bOK;
}

//------------------------------------------------------------------------------
// process
//  if temperature, rain and biome exist for a single location, add biome value
//  to the set referenceed by (temp, rain)
//  and register min and max values of rain and temp
// 
void TempRainBiome::process() {
    printf("processing...\n");
    m_mapTRBAcc.clear();
    //
    uchar  **pBData = m_pQMRBiome->getData();
    
    m_fMinTemp = fPosInf;
    m_fMaxTemp = fNegInf;
    m_fMinRain = fPosInf;
    m_fMaxRain = fNegInf;

    printf("Loooking for extremes in [%f,%f](%f)x[%f,%f](%f)\n", m_dLonMin, m_dLonMax, m_dDLon, m_dLatMin, m_dLatMax, m_dDLat);
    for (double dLat = m_dLatMin; dLat < m_dLatMax; dLat += m_dDLat) {
        int i = (int)round(m_pQMRBiome->Lat2Y(dLat));
        printf("row %4d ...\r", i);
        for (double dLon = m_dLonMin; dLon < m_dLonMax; dLon += m_dDLon) {
            int j = (int)round(m_pQMRBiome->Lon2X(dLon));
            uchar ud = pBData[i][j];
            if (ud != VAL_OCEAN) {
                // can't use grid coords, because Biome and Temp maps are probably of different size
                float fTemp = m_pQMRTemp->getDValue(dLon, dLat);
                float fRain = m_pQMRRain->getDValue(dLon, dLat);
                
                if ((dLon > 44.2) && (dLon < 44.4) && (dLat > 42.6) && (dLat < 42.8)) {
                    printf("(%f,%f) -> T %f, R %f\n", dLon, dLat, fTemp, fRain); 
                }
                if (isfinite(fTemp) && isfinite(fRain)) {
                    if (fTemp < m_fMinTemp) {
                        m_fMinTemp = fTemp;
                    }
                    if (fTemp > m_fMaxTemp) {
                        m_fMaxTemp = fTemp;
                    }
                    if (fRain < m_fMinRain) {
                        m_fMinRain = fRain;
                        if (m_fMinRain < 0) {
                            printf("Neg rain %f at %f,%f\n", m_fMinRain, dLon, dLat);
                        }
                    }
                    if (fRain > m_fMaxRain) {
                        m_fMaxRain = fRain;
                    }
                    // save biome value for temperature and rain
                    FPOINT pr(fTemp, fRain);
                    m_mapTRBAcc[pr].insert(ud);
                }
            }
        }
    }
    m_bVerbose=true;
    if (m_bVerbose) {
        printf("Data: %dx%d\n", m_pQMRBiome->getNRLon(), m_pQMRBiome->getNRLat());
        printf("TempRange: [%f,%f]\n", m_fMinTemp, m_fMaxTemp);
        printf("RainRange: [%f,%f]\n", m_fMinRain, m_fMaxRain);
        printf("NumEls: %zd\n", m_mapTRBAcc.size());
        //display(m_mapTRBAcc);
    }
}


//------------------------------------------------------------------------------
// collectMultiples
//   ??????????
//
int TempRainBiome::collectMultiples() {
    m_mapMultiple.clear();
    TRBACC::iterator it;
    for (it = m_mapBinned.begin(); it != m_mapBinned.end(); ++it) {
        if (it->second.size() > 1) {
            FPOINT pr( it->first.first, it->first.second);
            
            SET_UCHARS::iterator itt;
            for (itt = it->second.begin(); itt != it->second.end(); ++itt) {
                m_mapMultiple[pr].insert(*itt);
            }
        }
        
    }
    return m_mapMultiple.size();
}

//------------------------------------------------------------------------------
// createBins
//   create new map (temp,rain)=>set(uchar) where temp and rain only have
//   values for their respective bins.
//   returns maximum value
//
int TempRainBiome::createBins(SET_UCHARS &su) {
    int iBMax = -1;
    m_mapBinned.clear();

    TRBACC::iterator it;
    for (it = m_mapTRBAcc.begin(); it != m_mapTRBAcc.end(); ++it) {
        float fT = floor(it->first.first/m_fTempBinSize)*m_fTempBinSize;
        float fR = floor(it->first.second/m_fRainBinSize)*m_fRainBinSize;
        FPOINT pr(fT, fR);

        SET_UCHARS::iterator itt;
        //   printf("adding to (%f,%f) \n", fT, fR);
        for (itt = it->second.begin(); itt != it->second.end(); ++itt) {
            unsigned char uc = *itt;
            m_mapBinned[pr].insert(uc);
            su.insert(uc);
            if (uc > iBMax) {
                iBMax = uc;
            }
        }
    }

    if (m_bVerbose) {
        printf("Num biomes: %zd: ", su.size());
        displaySet(su);
        printf(" -> Max: %d\n", iBMax);
    }
    return iBMax;
}

//------------------------------------------------------------------------------
// fillTableData
//   create an array and fill with data from the map of binned values 
//   if a (temp,rain)-pair has more than 1 biome value, VAL_MULTI is used
//
void TempRainBiome::fillTableData(int iW, int iH) {
    if (m_aaucData != NULL) {
        QMapUtils::deleteArray(iW, iH, m_aaucData);
    }
    m_aaucData = QMapUtils::createArray(iW, iH, VAL_UNDEF);
    TRBACC::iterator it2;
    for (it2 = m_mapBinned.begin(); it2 != m_mapBinned.end(); ++it2) {
        
        int iX = (int) ((it2->first.first-m_fMinBinTemp)/m_fTempBinSize);
        int iY = (int) ((it2->first.second-m_fMinBinRain)/m_fRainBinSize);
        if ((iX >= iW) || (iY >= iH) || (iX < 0) || (iY < 0)) {
            printf("blast (%f,%f) -> (%d,%d)\n", it2->first.first, it2->first.second, iX, iY);fflush(stdout);
        }
        if (it2->second.size() > 1) {
            m_aaucData[iY][iX] = VAL_MULTI;
            //            printf("multi at x:%d, y:%d\n", iX, iY);
        } else {
            if (*(it2->second.begin()) == VAL_OCEAN) {
                printf("ocean at %d,%d\n", iY, iX);
            }
            m_aaucData[iY][iX] = *(it2->second.begin());
        }
    }

}

//------------------------------------------------------------------------------
// collectNeighborValues
//   collect values of neighbors sitting on a square of size 2*d
//   could be refinded to circular neighborhood
//
void TempRainBiome::collectNeighborValues(int iX, int iY, int iW, int iH, int iD, UIMAP &B) {
    int iXMin = iX - iD;
    int iYMin = iY - iD;
    int iXMax = iX + iD;
    int iYMax = iY + iD;
    
    int x = iXMin;
    int y = iYMin;
    while (x < iXMax) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                //                B[u]++;
                B[u] += weights[u];
            }
        }
        ++x;
    }
    while (y < iYMax) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                //                B[u]++;
                B[u] += weights[u];
           }
        }
        ++y;
    }
    while (x > iXMin) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                //                B[u]++;
                B[u] += weights[u];
            }
        }
        --x;
    }
    while (y > iYMin) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                //                B[u]++;
                B[u] += weights[u];
            }
        }
        --y;
    }

}

//------------------------------------------------------------------------------
// uniquify
//   tries to select a value from a set of biome values by
//   choosing the one occurring most often in a neighborhood
//   
void TempRainBiome::uniquify(int iW, int iH, int iD) {
    // loop through multiples
    int iC = 0;
    int iMax = (iW >iH)?iW:iH;
    TRBACC::iterator it2;
    for (it2 = m_mapMultiple.begin(); it2 != m_mapMultiple.end(); ++it2) {
        // get point P
        int iX = (int) ((it2->first.first-m_fMinBinTemp)/m_fTempBinSize);
        int iY = (int) ((it2->first.second-m_fMinBinRain)/m_fRainBinSize);
        //        printf("(%f,%f)[%d,%d]:", it2->first.first,it2->first.second, iX, iY);
        // get value set A
        SET_UCHARS A = it2->second;
        //@@printf("Set A:");
        //@@displaySet(A);
        UIMAP B;
        iD = 1;
        // look at ever increasing neighborhoods
        // until we get a unique value (or neighborhood exceeds image size)
        do {
            //@@printf(" iD %d ", iD);
            // clear previous values (must be done otherwise B tends to grow bigger, 
            // diminishing the chance to find a singleton intersection
            //B.clear();
            // in array, collect values of neighbours of P (except for val_none)-> set B
            collectNeighborValues(iX, iY, iW, iH, iD, B);
            //@@printf(", Set B:");
            //@@displaySet(B);
            
            int   iNMax=-1;
            // find highest occurrence in B of elements in A
            SET_UCHARS::iterator ii;
            for (ii = A.begin(); ii != A.end(); ++ii) {
                //@@printf (" Cur:(%d,%d)", *ii, B[*ii]);                
                if (B[*ii] > iNMax) {
                    iNMax = B[*ii];
  
                }
            }
  
            // move all elements of highest occurrence to C
            SET_UCHARS C;
            for (ii = A.begin(); ii != A.end(); ++ii) {
                if (B[*ii] == iNMax) {
                    C.insert(*ii);
                }
            }
                
            // if C is a singleton, then set m_mapBinned[P] = C
            //@@printf(", Set C:");
            //@@displaySet(C);
            //@@printf("\n");
            iC = C.size();
            if (C.size() == 1) {
                m_mapBinned[it2->first] = C;
                m_aaucData[iY][iX] = *(C.begin());
            }
            iD++; 
            
        } while ((iC != 1) && (iD <= iMax));
        
    }
    
}

//------------------------------------------------------------------------------
// collectNeighborValuesRing
//   collect values of neighbors sitting on a square of size 2*d
//   could be refinded to circular neighborhood
//
void TempRainBiome::collectNeighborValuesRing(int iX, int iY, int iW, int iH, int iD, SET_UCHARS &B) {
    int iXMin = iX - iD;
    int iYMin = iY - iD;
    int iXMax = iX + iD;
    int iYMax = iY + iD;
    
    int x = iXMin;
    int y = iYMin;
    while (x < iXMax) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                B.insert(u);
            }
        }
        ++x;
    }
    while (y < iYMax) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                B.insert(u);
            }
        }
        ++y;
    }
    while (x > iXMin) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                B.insert(u);
            }
        }
        --x;
    }
    while (y > iYMin) {
        if ((x >= 0) && (x < iW) && (y >= 0) && (y < iH)) {
            uchar u = m_aaucData[y][x];
            if ((u != VAL_UNDEF) && (u != VAL_MULTI)) {
                //                printf("added %d\n", u);
                B.insert(u);
            }
        }
        --y;
    }

}

//------------------------------------------------------------------------------
// uniquifyRing
//   tries to select a value from a set of biome values by
//   intersecting it with the set of values from ever growing neighborhoods
//   until intersection is a singleton or maximum neighborhood has been reached.
//   
void TempRainBiome::uniquifyRing(int iW, int iH, int iD) {
    // loop through multiples
    int iC = 0;
    int iMax = (iW >iH)?iW:iH;
    TRBACC::iterator it2;
    for (it2 = m_mapMultiple.begin(); it2 != m_mapMultiple.end(); ++it2) {
        // get point P
        int iX = (int) ((it2->first.first-m_fMinBinTemp)/m_fTempBinSize);
        int iY = (int) ((it2->first.second-m_fMinBinRain)/m_fRainBinSize);
        //        printf("(%f,%f)[%d,%d]:", it2->first.first,it2->first.second, iX, iY);
        // get value set A
        SET_UCHARS A = it2->second;
        //printf("Set A:");
        //displaySet(A);
        SET_UCHARS B;
        iD = 1;
        do {
            //   printf(" uniquify loop with %d ", iD);
            // clear previous values (must be done otherwise B tends to grow bigger, 
            // diminishing the chance to find a singleton intersection
            B.clear();
            // in array, collect values of neighbours of P (except for val_none)-> set B
            collectNeighborValuesRing(iX, iY, iW, iH, iD, B);
            //printf(", Set B:");
            //displaySet(B);
            
            // set C to the intersection of A and B
            SET_UCHARS C;
            std::insert_iterator<SET_UCHARS> it(C, C.begin());
            std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), it);
            // if C is a singleton, then set m_mapBinned[P] = C
            //printf(", Set C:");
            //displaySet(C);
            //printf("\n");
            iC = C.size();
            if (C.size() == 1) {
                m_mapBinned[it2->first] = C;
                m_aaucData[iY][iX] = *(C.begin());
            }
            iD++; 
            
        } while ((iC != 1) && (iD <= iMax));
        
    }
    
}

//------------------------------------------------------------------------------
// markNeighbors
//   mark empty neighbors of a point with with the point's color
//
int TempRainBiome::markNeighbors(int iW, int iH) {
    int iMarked = 0;
    m_mapMultiple.clear();
    // m_mapBinned.clear();
    uchar **aaucMarks = QMapUtils::createArray(iW, iH, VAL_UNDEF);
    // for all points

    for (int iY = 0; iY < iH; ++iY) {
        for (int iX = 0; iX < iW; ++iX) {
            uchar u = m_aaucData[iY][iX];
            if (u != VAL_UNDEF) {
                // if point is not empty, loop through neighbors
                for (int j = 0; j < NUM_NEIGH; ++j) {
                    int iU = iX + aiNeighborX[j];
                    int iV = iY + aiNeighborY[j];
                    // only consider legal coordinates
                    if ((iU >= 0) && (iU < iW) && (iV >= 0) && (iV < iH)) {
                        // if neighbor is empty ...
                        if (m_aaucData[iV][iU] == VAL_UNDEF) {
                            iMarked++;
                                
                            // ... and unmarked ...
                            if (aaucMarks[iV][iU] == VAL_UNDEF) {
                                // ... mark it with point's color
                                aaucMarks[iV][iU] = u;
                            } else {

                                // ... otherwise, mark it as multiple and add point to multiples
                                FPOINT pr(m_fMinBinTemp+m_fTempBinSize*iU, m_fMinBinRain+m_fRainBinSize*iV);
                                if (aaucMarks[iV][iU] != VAL_MULTI) {
                                    m_mapMultiple[pr].insert(aaucMarks[iV][iU]);
                                }
                                m_mapMultiple[pr].insert(u);
                                aaucMarks[iV][iU] = VAL_MULTI;
                            }
                        }
                    }
                }
            }
        }
    }

    // now copy all singly marked points to original
    for (int iY = 0; iY < iH; ++iY) {
        for (int iX = 0; iX < iW; ++iX) {
            if (aaucMarks[iY][iX] != VAL_UNDEF) {
                FPOINT pr(m_fMinBinTemp+m_fTempBinSize*iX, m_fMinBinRain+m_fRainBinSize*iY);
                //printf("setting (%d,%d) to %d\n", iX, iY, aaucData2[iY][iX]);
                m_mapBinned[pr].insert(aaucMarks[iY][iX]);
                // update array
                m_aaucData[iY][iX] = aaucMarks[iY][iX];
            }
        }
    }    
    QMapUtils::deleteArray(iW, iH, aaucMarks);

    return iMarked;
}


//-------------------------------------------------------
// createBiomes
//   - create biome from given rain and temperature data
//   - rain & temperature arrays must have format of m_pQMHIntersection
//   - for each cell the ran & temperature values are used as 
//     indexes into the rb-table
//   - for undefined or nan rain/temperature values the biome value 
//     is set to VAL_OCEAN
//
uchar **TempRainBiome::createBiomes(float **pTData, float **pRData, float fDeltaTemp, float fScaleRain) {
    m_iMaxBiomeR = m_iMaxBiome;
    biomestat mapBiomes;

    printf("[createBiomes] Need array %dx%d=%d\n", m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, m_pQMHIntersection->m_iWidth*m_pQMHIntersection->m_iHeight);
    uchar **aaucData = QMapUtils::createArray(m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, VAL_UNDEF);

    if (m_bEqualGrids) {
        createBiomesGrid(pTData, pRData, fDeltaTemp, fScaleRain, aaucData, mapBiomes);
    } else {
        createBiomesCoord(pTData, pRData, fDeltaTemp, fScaleRain, aaucData, mapBiomes);
    }

   if (m_bVerbose) {
        printf("[createBiomes] Biome statistics: \n");
        std::map<uchar,int>::iterator it;
        for (it = mapBiomes.begin(); it != mapBiomes.end(); it++) {
            printf("  %d -> %d\n", it->first, it->second);
        }
    }
    return aaucData;
}

//-------------------------------------------------------
// createBiomesCoord
//   - uses coordinates given by QMHIntersection
//   - create biome from given rain and temperature data
//   - rain & temperature arrays must have format of m_pQMHIntersection
//   - for each cell the ran & temperature values are used as 
//     indexes into the rb-table
//   - for undefined or nan rain/temperature values the biome value 
//     is set to VAL_OCEAN
//
void TempRainBiome::createBiomesCoord(float **pTData, float **pRData, float fDeltaTemp, float fScaleRain, uchar **aaucData, biomestat &mapBiomes) {
    

    for (unsigned int i = 0; i < m_pQMHIntersection->m_iHeight; i++) {
        double dLat = m_pQMHIntersection->Y2Lat(i);
        for (unsigned int j = 0; j < m_pQMHIntersection->m_iWidth; j++) {
            double dLon = m_pQMHIntersection->X2Lon(j);

            float fTemp = m_pQMRTemp->getDValue(dLon, dLat);
            float fRain = m_pQMRRain->getDValue(dLon, dLat);
            if (isfinite(fTemp) && isfinite(fRain)) {
            

                // can't use grid coordinates
                fTemp += fDeltaTemp;
                fRain *= fScaleRain;


                if (fTemp < m_fMinBinTemp) {
                    fTemp = m_fMinBinTemp;
                }
                if (fRain < m_fMinBinRain) {
                    fRain = m_fMinBinRain;
                }
                
                int iTemp =  (int)((fTemp - m_fMinBinTemp)/m_fTempBinSize);
                int iRain =  (int)((fRain - m_fMinBinRain)/m_fRainBinSize);
                if (iRain >= m_iMaxRain) {
                    iRain = m_iMaxRain-1;
                }
                if (iTemp >= m_iMaxTemp) {
                    iTemp = m_iMaxTemp-1;
                }
                
                if ((iRain >= 0) && (iTemp >= 0)) {
                    aaucData[i][j] = m_aaucData[iRain][iTemp];
                    if (aaucData[i][j] > m_iMaxBiomeR) {
                        m_iMaxBiomeR = aaucData[i][j];
                    }
                } else {
                    printf("[createBiomesCoord] Bad Index (r,t)=(%d,%d) at (%d,%d)\n", iTemp, iRain, j, i);
                    printf("  fTemp %f, minbin %f, binsize %f\n", fTemp, m_fMinBinTemp, m_fTempBinSize);
                    printf("  fRain %f, minbin %f, binsize %f\n", fRain, m_fMinBinRain, m_fRainBinSize);
                }


            } else {
                aaucData[i][j] = VAL_OCEAN;
            }
            mapBiomes[aaucData[i][j]]++;
        }
    }
}

//-------------------------------------------------------
// createBiomes
//   - uses grid coordinates: only to be used if temp and Rain maps have same size & delta
//   - create biome from given rain and temperature data
//   - rain & temperature arrays must have format of m_pQMHIntersection
//   - for each cell the ran & temperature values are used as 
//     indexes into the rb-table
//   - for undefined or nan rain/temperature values the biome value 
//     is set to VAL_OCEAN
//
void TempRainBiome::createBiomesGrid(float **pTData, float **pRData, float fDeltaTemp, float fScaleRain, uchar **aaucData, biomestat &mapBiomes) {
    
    for (unsigned int i = 0; i < m_pQMHIntersection->m_iHeight; i++) {
        for (unsigned int j = 0; j < m_pQMHIntersection->m_iWidth; j++) {

            float fTemp = m_pQMRTemp->getDValue(j, i);
            float fRain = m_pQMRRain->getDValue(j, i);
            if (isfinite(fTemp) && isfinite(fRain)) {
                // using grid coordinates
                fTemp += fDeltaTemp;
                fRain *= fScaleRain;


                if (fTemp < m_fMinBinTemp) {
                    fTemp = m_fMinBinTemp;
                }
                if (fRain < m_fMinBinRain) {
                    fRain = m_fMinBinRain;
                }

                int iTemp =  (int)((fTemp - m_fMinBinTemp)/m_fTempBinSize);
                int iRain =  (int)((fRain - m_fMinBinRain)/m_fRainBinSize);
                if (iRain >= m_iMaxRain) {
                    iRain = m_iMaxRain-1;
                }
                if (iTemp >= m_iMaxTemp) {
                    iTemp = m_iMaxTemp-1;
                }
                
                if ((iRain >= 0) && (iTemp >= 0)) {
                    aaucData[i][j] = m_aaucData[iRain][iTemp];
                    if (aaucData[i][j] > m_iMaxBiomeR) {
                        m_iMaxBiomeR = aaucData[i][j];
                    }
                } else {
                    printf("[createBiomesGrid] Bad Index (r,t)=(%d,%d) at (%d,%d)\n", iTemp, iRain, j, i);
                    printf("  fTemp %f, minbin %f, binsize %f\n", fTemp, m_fMinBinTemp, m_fTempBinSize);
                    printf("  fRain %f, minbin %f, binsize %f\n", fRain, m_fMinBinRain, m_fRainBinSize);
                }

            } else {
                aaucData[i][j] = VAL_OCEAN;
            }
            mapBiomes[aaucData[i][j]]++;
        }
    }
}

//-------------------------------------------------------
// writeTRBTable
//
bool TempRainBiome::writeTRBTable(char *pOutput, int iW, int iH) {
    bool bOK = false;



    FILE *fOut = fopen(pOutput, "wb");
    if (fOut != NULL) {
        bOK = true;
        QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_UCHAR, 
                                          m_fMinBinTemp, m_fMaxBinTemp+m_fTempBinSize/2, m_fTempBinSize,
                                          m_fMinBinRain, m_fMaxBinRain+m_fRainBinSize/2, m_fRainBinSize,
                                          "Biome", "Temp", "Rain",true,true);
        bOK = pQMH->addHeader(fOut);
        if (bOK) {
            printf("writing (%dx%d), fill with %dx%d\n", iW, iH, pQMH->m_iWidth, pQMH->m_iHeight); 
            bOK = QMapUtils::writeArray(fOut, iW, iH, m_aaucData);
            /*
            for (int iY = 0; bOK && (iY < iH); iY++) {
                int iWritten = fwrite(m_aaucData[iY], 1, iW, fOut);
                if (iWritten != iW) {
                    printf("only wrote %d chars at line %d\n", iWritten, iY);
                    bOK = false;
                }
            }
            */
        
            fclose(fOut);

        } else {
            printf("Couldn't write header to %s\n", pOutput);
        }
        delete pQMH;

    } else {
        printf("Couldn't open output file [%s]\n", pOutput);
    }
    return bOK;
}

//-------------------------------------------------------
// writeBiomeFile
//
bool TempRainBiome::writeBiomeFile(char *pOutput, uchar **aaucDataR) {
    bool bOK = false;
    strcpy(m_pQMHIntersection->m_sVName, "Biome");
    FILE *fOut = fopen(pOutput, "wb");
    if (fOut != NULL) {
        m_pQMHIntersection->addHeader(fOut);
        bOK = true;
        bOK = QMapUtils::writeArray(fOut, m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, aaucDataR);
        /*
        for (int iY = 0; bOK && (iY < m_pQMHIntersection->m_iHeight); iY++) {
            int iWritten = fwrite(aaucDataR[iY], 1, m_pQMHIntersection->m_iWidth, fOut);
            if (iWritten != m_pQMHIntersection->m_iWidth) {
                printf("only wrote %d chars at line %d\n", iWritten, iY);
                bOK = false;
            }
        }
        */
        fclose(fOut);
    } else {
        printf("Couldn't open output file [%s]\n", pOutput);
    }
    return bOK;
}


//-------------------------------------------------------
// createBiomes
//   set new rain and temperature files and create biomes
//   if either of the filenames iss empty, use corresponding
//   data from makeTable
//
void TempRainBiome::createBiomes(char *pTableFile, char *pOutputFile, const char *pSuff, float fDeltaTemp, float fScaleRain) {
    int iType;

    char sNameImg[MAX_PATH];
    //    char sNamePng[MAX_PATH];

    printf("[createBiomes] opening TRB table [%s]\n", pTableFile);
    QMapReader<uchar> *pQMRTRB = dynamic_cast<QMapReader<uchar>*>(QMapUtils::createValReader(pTableFile, true, &iType));
    printf("   type %d (%s)\n", iType, QMapHeader::getTypeName(iType));
    if (iType == QMAP_TYPE_UCHAR) {

        m_fTempBinSize = pQMRTRB->getDLon(); // table Lon <-> temp
        m_fRainBinSize = pQMRTRB->getDLat(); // table Lat <-> rain

  
        m_fMinTemp = pQMRTRB->getLonMin(); // table Lon <-> temp
        m_fMaxTemp = pQMRTRB->getLonMax(); // table Lon <-> temp;
        m_fMinRain = pQMRTRB->getLatMin(); // table Lat <-> rain
        m_fMaxRain = pQMRTRB->getLatMax(); // table Lat <-> rain

        
        m_fMinBinTemp =  floor(m_fMinTemp/m_fTempBinSize)*m_fTempBinSize;
        m_fMaxBinTemp =  floor(m_fMaxTemp/m_fTempBinSize)*m_fTempBinSize;
        m_fMinBinRain =  floor(m_fMinRain/m_fRainBinSize)*m_fRainBinSize;
        m_fMaxBinRain =  floor(m_fMaxRain/m_fRainBinSize)*m_fRainBinSize;
   
        printf("[createBiomes]temp [%f,%f](%f), rain [%f,%f](%f)\n", m_fMinBinTemp, m_fMaxBinTemp, m_fTempBinSize, m_fMinBinRain, m_fMaxBinRain, m_fRainBinSize);

        // size of binned image
        m_iMaxTemp = (int)(1+ceil((m_fMaxBinTemp - m_fMinBinTemp)/m_fTempBinSize));
        m_iMaxRain = (int)(1+ceil((m_fMaxBinRain - m_fMinBinRain)/m_fRainBinSize));

        
        m_aaucData =pQMRTRB->getData();
        uchar **aaucDataR = createBiomes(m_pQMRTemp->getData(), m_pQMRRain->getData(), fDeltaTemp, fScaleRain);
        m_aaucData = NULL; // don't delete this!!


        sprintf(sNameImg, "%s%s.qmap",pOutputFile, pSuff);
        //        sprintf(sNamePng, "%s%s.png",pOutputFile, pSuff);

        printf("[createBiomes] Writing biomes to %s\n", sNameImg);
        writeBiomeFile(sNameImg, aaucDataR);

        /*
        LookUp *pLU = new UCharLookUp(m_iMaxBiomeR);
        BinVisualizer<uchar> *pBV = new BinVisualizer<uchar>(aaucDataR,  m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, pLU);
        pBV->visualize(sNamePng);
        delete pBV;
        delete pLU;
        */

        QMapUtils::deleteArray(m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, aaucDataR);

    }
    delete pQMRTRB;
 
}

//-------------------------------------------------------
// checkClimateFile
//
QMapReader<float>* TempRainBiome::checkClimateFile(char *pClimateFile, int *piW, int *piH) {
    bool bOK = false;
    int iType = QMAP_TYPE_NONE;

    printf("[checkClimateFile] reading temp file %s\n", pClimateFile);
    QMapReader<float>* pQMRClim = dynamic_cast<QMapReader<float>*>(QMapUtils::createValReader(pClimateFile, true, &iType));
    if (pQMRClim != NULL) {
        if (iType == QMAP_TYPE_FLOAT) {
            bOK = true;
            *piW = pQMRClim->getNRLon();
            *piH = pQMRClim->getNRLat();
            printf("[checkClimateFile] Extracted data from %s (%dx%d)\n", pClimateFile, *piW, *piH);
        } else {
            printf("[checkClimateFile] Climate file [%s] should be of type %s but is %s(%d)\n", pClimateFile, QMapHeader::getTypeName(QMAP_TYPE_FLOAT), QMapHeader::getTypeName(iType), iType);
            bOK = false;
        }
    } else {
        printf("[checkClimateFile] Couldn't open climate file [%s]\n", pClimateFile);
        bOK = false;
    }
    if (!bOK) {
        if (pQMRClim != NULL) {
            delete pQMRClim;
            pQMRClim = NULL;
        }
    }

    return pQMRClim;
}

//-------------------------------------------------------
// checkFiles
//   set new rain and temperature files and create biomes
//   if either of the filenames iss empty, use corresponding
//   data from makeTable
//
void TempRainBiome::checkFiles(char *pNewTempFile, char *pNewRainFile, char *pOutputFile, const char *pSuff, float fDeltaTemp, float fScaleRain) {

    char sNameImg[MAX_PATH];
    //    char sNamePng[MAX_PATH];

    int iWT;
    int iHT;
    int iWR;
    int iHR;

    bool bOK = false;
    //resetIntersection();

    printf("new tempfile [%s]\n", pNewTempFile);
 
    if (*pNewTempFile != '\0') {
        delete m_pQMRTemp;

        m_pQMRTemp = checkClimateFile(pNewTempFile, &iWT, &iHT);
        if (m_pQMRTemp != NULL) {
            bOK = true;
        } else {
            bOK = false;
        }

    } else {
        printf("Using original for new temp\n");
        bOK = true;
    }

    if (bOK) {
        calcMapIntersection(m_pQMRTemp);
    }

    printf("--- result: %s\n", bOK?"ok":"BAD");


    printf("new rainfile [%s]\n", pNewRainFile);

    bOK = false;
    if (*pNewRainFile != '\0') {
        delete m_pQMRRain;
        m_pQMRRain = checkClimateFile(pNewRainFile, &iWR, &iHR);
        if (m_pQMRRain != NULL) {
            bOK = true;
        } else {
            bOK = false;
        }

    } else {
        printf("Using original for new rain\n");
        bOK = true;
    }

    if (bOK) {
        calcMapIntersection(m_pQMRRain);
    }
    printf("--- result: %s\n", bOK?"ok":"BAD");
    
    if (bOK) {
        if (m_pQMHIntersection != NULL) {
            delete m_pQMHIntersection;
            m_pQMHIntersection = NULL;
        }
        printf("Intersection 2:\n");
        printf("Lon: [%f,%f]:%f\n", m_dLonMin, m_dLonMax, m_dDLon);
        printf("Lat: [%f,%f]:%f\n", m_dLatMin, m_dLatMax, m_dDLat);

        m_pQMHIntersection = new QMapHeader(QMAP_TYPE_UCHAR,  
                                            m_dLonMin, m_dLonMax, m_dDLon,
                                            m_dLatMin, m_dLatMax, m_dDLat);

        printf("intersection size: %dx%d\n", m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight);


        sprintf(sNameImg, "%s%s.qmap",pOutputFile, pSuff);
        //        sprintf(sNamePng, "%s%s.png",pOutputFile, pSuff);
        

        printf("max bi : %d\n", m_iMaxBiome);
        
        
        uchar **aaucDataR = createBiomes(m_pQMRTemp->getData(),
                                           m_pQMRRain->getData(),
                                           fDeltaTemp, fScaleRain);
        /*
        LookUp *pLU = new UCharLookUp(m_iMaxBiome);
        BinVisualizer<uchar> *pBV = new BinVisualizer<uchar>(aaucDataR,  m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, pLU);
        pBV->visualize(sNamePng);
        delete pBV;
        delete pLU;
        */

        writeBiomeFile(sNameImg, aaucDataR);
        QMapUtils::deleteArray(m_pQMHIntersection->m_iWidth, m_pQMHIntersection->m_iHeight, aaucDataR);
        
    } else {
        printf("something is not ok\n");
    }
    
}



//------------------------------------------------------------------------------
// createStateVectors
//   create new map (temp,rain)=>set(uchar) where temp and rain only have
//   values for their respective bins.
//   returns maximum value
//
int TempRainBiome::createStateVectors() {
    int iBMax = -1;
    m_bsb.clear();
    uchar **pBData = m_pQMRBiome->getData();


    for (double dLat = m_dLatMin; dLat < m_dLatMax; dLat += m_dDLat) {
        int i = (int)round(m_pQMRBiome->Lat2Y(dLat));
        for (double dLon = m_dLonMin; dLon < m_dLonMax; dLon += m_dDLon) {
            int j = (int)round(m_pQMRBiome->Lon2X(dLon));
            uchar ud = pBData[i][j];
            if (ud != VAL_OCEAN) {
                float fTemp = m_pQMRTemp->getDValue(dLon, dLat);
                float fRain = m_pQMRRain->getDValue(dLon, dLat);

                int iT = (int)(1+ceil((fTemp - m_fMinBinTemp)/m_fTempBinSize));
                int iR = (int)(1+ceil((fRain - m_fMinBinRain)/m_fRainBinSize));

                
                loc p0(iT, iR);

                m_bsb[p0].add(ud);
                if (ud > iBMax) {
                    iBMax = ud;
                }
            }
        }
    }

    if (m_bVerbose) {
        printf("Num state vectors: %zd: ", m_bsb.size());
        printf(" -> Max biome: %d\n", iBMax);
    }
    return iBMax;
}



//-------------------------------------------------------------------------------------------------
// getRing
//   get coordinates of cells in a square ring of size iSize around p0
//   and collect them in vRing
//   ignore ring cells who lie outside thbounds (xmin,ymin)-(xmax,ymax)
//    
void getRing(loc p0, int i, std::vector<loc> &vRing, int xmin, int xmax, int ymin, int ymax) {
    vRing.clear();

    int x = p0.first-i;
    int y = p0.second-i;

    if (x >= xmin) {
        for (int k = 0; k < 2*i; k++) {
            if ((y >= ymin) && (y <ymax)) {
                vRing.push_back(loc(x, y));
            }
            y++;
        }
    }

    y = p0.second+i;
    if (y < ymax) {
        for (int k = 0; k < 2*i; k++) {
            if ((x >= xmin) && (x < xmax)) {
                vRing.push_back(loc(x, y));
            }
            x++;
        }
    }

    x = p0.first+i;
    if (x < xmax) {
        for (int k = 0; k < 2*i; k++) {
            if ((y >= ymin) && (y <ymax)) {
                vRing.push_back(loc(x, y));
            }
            y--;
        }
    }

    y = p0.second-i;
    if (y >= ymin) {
        for (int k = 0; k < 2*i; k++) {
            if ((x >= xmin) && (x <xmax)) {
                vRing.push_back(loc(x, y));
            }
            x--;
        }
    }


}

//-------------------------------------------------------------------------------------------------
// reduceAll
//   
void reduceAll(BStateVector **bsa, int tMax, int rMax, statevec &weights, uchar **aaucData) {
    for (int r = 0; r < rMax; r++) {
        for (int t = 0; t < tMax; t++) {
            BStateVector &bsv = bsa[r][t];
            //            printf("checking (%d,%d):", t, r);printbsv(bsv);printf("\n");

            int iRed = bsv.reduce(weights);
            if (iRed == 1) {
                //                printf("bsv at (%d,%d) has reduced:", t, r);printbsv(bsv);printf("\n");
                aaucData[r][t] = bsv.m_ucState;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
// selectUndefs
//   
void selectUndefs(BStateVector **bsa, int tMax, int rMax, statevec &weights, uchar **aaucData) {
    for (int r = 0; r < rMax; r++) {
        for (int t = 0; t < tMax; t++) {
            if (aaucData[r][t] == VAL_UNDEF) {
                BStateVector &bsv = bsa[r][t];
                //                printf("collapsing [%d,%d]:", t,r);printbsv(bsv);printf("\n");
                bsv.collapse(weights);
                aaucData[r][t] = bsv.m_ucState;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
// spread
//   
void spread(BStateVector **bsa, int tMax, int rMax, int iMaxSpread, BSBundle &bsb) {
    

    BSBundle::iterator iter;
    for (iter = bsb.begin(); iter != bsb.end(); iter++) {
        printf("\r At#%d,%d     ",iter->first.first, iter->first.second);
        if ((iter->first.second>=rMax) || (iter->first.first>=tMax)) {
            printf("Alarm: %d,%d (%d,%d)\n", iter->first.first, iter->first.second, tMax, rMax);fflush(stdout);
        } else {
        bsa[iter->first.second][iter->first.first].add(iter->second, 1);
        
        for (int k = 0; k < iMaxSpread; k++) {
            std::vector<loc> vRing;
            getRing(iter->first, k, vRing, 0, tMax, 0, rMax);
            for (unsigned int i=0; i < vRing.size(); i++) {
                bsa[vRing[i].second][vRing[i].first].add(iter->second, 1.0*(k+1));
            }
                 
        }
        }
        /*
        printf("After (%d,%d):", iter->first.first, iter->first.second);printbsv(iter->second);printf("\n");
        for (int i = 0; i < rMax; i++) {
            for (int j = 0; j < tMax; j++) {

                 printbsv( bsa[i][j]);printf(" ");
            }
            printf("\n");
        }
        */   
    }
}

//-------------------------------------------------------------------------------------------------
// bundle
//   
int bundle(BStateVector **bsa, int tMax, int rMax, BSBundle &bsb) {
    int iUndef = 0;
    bsb.clear();
    for (int i = 0; i < rMax; i++) {
        for (int j = 0; j < tMax; j++) {
            if (bsa[i][j].m_ucState == VAL_UNDEF) {
                iUndef++;
            }
            bsb[loc(j,i)] = bsa[i][j];
            bsa[i][j]=BStateVector();
        }
    }
    return iUndef;
}

const double EPS = 1E-8;

//------------------------------------------------------------------------------
// makeTable
//  create a (temp,rain)-> biome value lookup table.
//  1. create a binned version of the (rain, temp) => biom value map
//  2. create and draw image legend
//  3. create trb-table with sizes given by number of bins
//     and fill values from trb map (if singleton) or VAL_MULTI
//  4. stepwise eliminate values in trb-table by flooding empty cells
//     with values from neighbors. If empty cell has several neighbors,
//     use value occurring most often in growing neighborhood 
//     (loop 4 until steps have no effect. hopefully we have no more multiples)
//  5. draw trb-table, save it as binary file, save it as text file 
//  6. create biome file based on input rain & temp; draw, save as binary
//  
// Alternative to step 4:
//  4'.stepwise eliminate values in array by choosing from
//     intersections of values from cells in growing neighborhoods 
//  for this uniquifyRing should be used instead of uniquify
// 
void TempRainBiome::makeTable(float fTempBinSize, float fRainBinSize, bool bPExtend, int iSmooth, char *pOutputFile, char *pTransFile, bool bVerbose) {
    m_bVerbose = bVerbose;

    char sIMGtrb[MAX_PATH];
    //    char sPNGtrb[MAX_PATH];
    strcpy(sIMGtrb,   pOutputFile);
    strcat(sIMGtrb,   SUFF_TRBQ);
    //    strcpy(sPNGtrb,   pOutputFile);
    //    strcat(sPNGtrb,   SUFF_TRBI);

    if (bPExtend) {
        m_fMinRain = 0;
    }

    m_fTempBinSize = fTempBinSize;
    m_fRainBinSize = fRainBinSize;

    ValueMapper<uchar> *pSM = NULL;
    if (*pTransFile != '\0') {
        printf("SImple mapper with %s\n", pTransFile);
        pSM = new SimpleMapper<uchar>(pTransFile, 0xff);
    } else {
        pSM = new IdentityMapper<uchar>();
    }

    // calculate extreme values for bins
    time_t t0 = clock();

    double dTempPh = 0; //QMapHeader::getPhase(m_fMinTemp, m_fTempBinSize);
    double dRainPh = 0; //QMapHeader::getPhase(m_fMinRain, m_fRainBinSize);
    printf("Phases Temp %f, Rain %f\n", dTempPh, dRainPh);

    m_fMinBinTemp = QMapHeader::getNextGridPoint(dTempPh, m_fTempBinSize, m_fMinTemp-m_fTempBinSize/2) - m_fTempBinSize;
    m_fMaxBinTemp = QMapHeader::getNextGridPoint(dTempPh, m_fTempBinSize, m_fMaxTemp+m_fTempBinSize/2) + m_fTempBinSize;
    m_fMinBinRain = QMapHeader::getNextGridPoint(dRainPh, m_fRainBinSize, m_fMinRain-m_fRainBinSize/2) - m_fRainBinSize;
    m_fMaxBinRain = QMapHeader::getNextGridPoint(dRainPh, m_fRainBinSize, m_fMaxRain+m_fRainBinSize/2) + m_fRainBinSize;

    // size of binned image
    m_iMaxTemp = (int)(1+(m_fMaxBinTemp-m_fMinBinTemp+EPS)/m_fTempBinSize);
    m_iMaxRain = (int)(1+(m_fMaxBinRain-m_fMinBinRain+EPS)/m_fRainBinSize);
    printf("Temp Extremes: %.9f - %.9f -> %.9f x %d\n", 
           m_fMinBinTemp, m_fMaxBinTemp, m_fMaxBinTemp-m_fMinBinTemp, m_iMaxTemp);
    printf("Rain Extremes: %.9f - %.9f -> %.9f x %d\n", 
           m_fMinBinRain, m_fMaxBinRain, m_fMaxBinRain-m_fMinBinRain, m_iMaxRain);
    unsigned int iNumTotal = m_iMaxTemp*m_iMaxRain;
    printf("Temp: [%f, %f] (%d bins); Rain: [%f, %f] (%d bins)\n", 
           m_fMinBinTemp, m_fMaxBinTemp, m_iMaxTemp, m_fMinBinRain, m_fMaxBinRain, m_iMaxRain); 
    printf("Image: %dx%d\n", m_iMaxTemp, m_iMaxRain);


    if (iSmooth == 0) {
        // standard version
 
        // create binned version
        SET_UCHARS su;
        m_iMaxBiome = createBins(su);
        printf("bMax : %d \n", m_iMaxBiome);
        printf("Num Binned:%zd\n", m_mapBinned.size());

        // collect map entries with non singleton sets in m_mapMultiple
        int iNMult = collectMultiples();
        int iNMult2 = -1;
        iNMult2 = iNMult;
        if (m_bVerbose) {
            printf("----------------\n");
            //        display(m_mapMultiple);
        }
      
        // fill m_aaucData with biome values (or VAL_MULTI) 
        fillTableData(m_iMaxTemp, m_iMaxRain);


        printf("initially binned: %zd/%d (%d multiples)\n", m_mapBinned.size(), iNumTotal,  iNMult);
            
        // try to eliminate  
        uniquify(m_iMaxTemp, m_iMaxRain,1);
        iNMult = collectMultiples();
        if (m_bVerbose) {
            printf("iNMult %d, iNMult2 %d\n", iNMult, iNMult2);
        }

        int iCount = 0;
        unsigned int iNBPrev = 0;
        printf("Before loop: binned:%zd\n", m_mapBinned.size());
        while ((iNBPrev != m_mapBinned.size()) && (m_mapBinned.size() < iNumTotal)) {
            iNBPrev = m_mapBinned.size();
            markNeighbors(m_iMaxTemp, m_iMaxRain);
            uniquify(m_iMaxTemp, m_iMaxRain,1);
            iNMult = collectMultiples();
            printf("\r[%d] binned: %zd/%d (%zd multiples)", iCount, m_mapBinned.size(), iNumTotal,  m_mapMultiple.size());fflush(stdout);
            iCount++;
        }
        printf("\n");
        if (iNBPrev == m_mapBinned.size()) {
            iNMult = collectMultiples();
            printf("Couldn't eliminate multiples (%d)\n", iNMult);
        }
        time_t t1 = clock();
        printf("used %ld\n", t1-t0);
    } else {
        
        //"statevector version"

        m_iMaxBiome = createStateVectors();
        printf("bMax : %d\n", m_iMaxBiome);

        if (m_aaucData != NULL) {
            QMapUtils::deleteArray(m_iMaxTemp, m_iMaxRain, m_aaucData);
        }
        m_aaucData = QMapUtils::createArray(m_iMaxTemp, m_iMaxRain, VAL_UNDEF);

        BStateVector **aabsv = new BStateVector*[m_iMaxRain];
        for (int i = 0; i < m_iMaxRain; i++) {
            aabsv[i] = new BStateVector[m_iMaxTemp];
        }

        statevec weights;
        for (uchar u = 0; u < m_iMaxBiome; u++) {
            weights[u] = 1;
        }

        int iUndefP = iNumTotal;
        int iUndef = iUndefP;
        int iSpread = iSmooth;
        int iCount = 0;
        do {
            iUndefP = iUndef;

            spread(aabsv, m_iMaxTemp, m_iMaxRain, iSpread, m_bsb);

            reduceAll(aabsv, m_iMaxTemp, m_iMaxRain, weights, m_aaucData);


            iUndef = bundle(aabsv, m_iMaxTemp, m_iMaxRain, m_bsb);
            //        showBundle(bsb);
            printf("\n[%d] undefs: %d/%d\n", iCount, iUndef, iNumTotal);fflush(stdout);
            iCount++;

        } while ((iUndef > 0) && (iUndef < iUndefP));
        
        printf("Exited with %d undefs\n", iUndef);

        if (iUndef > 0) {
            spread(aabsv, m_iMaxTemp, m_iMaxRain, iSpread, m_bsb);
            reduceAll(aabsv, m_iMaxTemp, m_iMaxRain, weights, m_aaucData);
            spread(aabsv, m_iMaxTemp, m_iMaxRain, iSpread, m_bsb);

            selectUndefs(aabsv, m_iMaxTemp, m_iMaxRain, weights, m_aaucData);
        }

        for (int i = 0; i < m_iMaxRain; i++) {
            delete[] aabsv[i];
        }
        delete[] aabsv;

        
    }    


    // check multiply occupied
    int iCCCC = 0;
    for (int i = 0; i< m_iMaxRain; i++) {
        for (int j = 0; j< m_iMaxTemp; j++) {
            if (m_aaucData[i][j] == VAL_MULTI) {
                iCCCC++;
            }
        }
    }
    printf("found %d multiply occupied cells\n", iCCCC);


    // write table
    mapTRBTable(pSM);
    writeTRBTable(sIMGtrb, m_iMaxTemp, m_iMaxRain);

    printf("binned : %zd els\n", m_mapBinned.size());
    // display(m_mapBinned);
    

    if (pSM != NULL) {
        delete pSM;
    }

    m_mapBinned.clear();
    m_mapMultiple.clear();
}

//-------------------------------------------------------------------------------------------------
// mapTRBTable
//   
void TempRainBiome::mapTRBTable(ValueMapper<uchar> *pVM) {
    for (int i = 0; i < m_iMaxRain; i++) {
        for (int j = 0; j < m_iMaxTemp; j++) {
            m_aaucData[i][j] = pVM->mapValue(m_aaucData[i][j]);
        }
    }
}
