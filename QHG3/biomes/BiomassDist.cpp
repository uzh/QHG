#include <map>
#ifdef OMP
#include <omp.h>
#endif
#include "types.h"
#include "utils.h"
#include "LineReader.h"
#include "TrinaryFunc.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "BiomassDist.h"

const uchar VAL_OCEAN = 0x00;
const uchar VAL_UNDEF = 0xfe;


//------------------------------------------------------------------------------
// constructor
//
BiomassDist::BiomassDist(char *pBiomeBaseFile)
    : m_pQMRBiomeBase(NULL),
      m_bReady(false),
      m_pPR(NULL),
      m_pfuncLocDep(NULL),
      m_aadMass(NULL),
      m_dLastLat(dNaN),
      m_iLastY(-1),
      m_pQMH(NULL) {

  
    m_pQMRBiomeBase = new QMapReader<uchar>(pBiomeBaseFile); 

   
};

//------------------------------------------------------------------------------
// destructor
//
BiomassDist::~BiomassDist() {
    /*

    */
    if (m_pPR != NULL) {
        delete m_pPR;
    }
    
    if (m_pQMRBiomeBase != NULL) {
        delete m_pQMRBiomeBase;
    }
    
    if (m_pQMH != NULL) {
        delete m_pQMH;
    }
}

//------------------------------------------------------------------------------
// setParams
//
bool BiomassDist::setParams(TrinaryFunc *pfuncLocDep,  char *pPercentageFile) {
   
    m_pfuncLocDep = pfuncLocDep;
    if (m_pPR != NULL) {
        delete m_pPR;
    }
    m_pPR = new PercentageReader(pPercentageFile, true);
    if (m_pPR != NULL) {
        m_bReady = m_pPR->isReady();
    }
    return m_bReady;
}

//------------------------------------------------------------------------------
// setParams
//
bool BiomassDist::setParams(TrinaryFunc *pfuncLocDep,  PercentageReader *pPR) {
   
    m_pfuncLocDep = pfuncLocDep;
    if (m_pPR != NULL) {
        delete m_pPR;
    }
    m_pPR = pPR;
    if (m_pPR != NULL) {
        m_bReady = m_pPR->isReady();
    }
    return m_bReady;
}


//------------------------------------------------------------------------------
// calcDensity
//  Calculate density of mass value whose dependence is given by funcLocDep for a
//  plant whose area coverage for a given biome value is given by 
//  mapPercentage (biome value => percentage)
//
// Formula for cell area derived from "Zone surface" = 2*M_PI*R*(sin(Lat1)-sin(Lat2))
//   a cell is a fraction of a zone.
//  
int BiomassDist::calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass,  VEC_STRINGS &vSpecies) {
    int iResult = m_bReady?0:-1;
    
    m_mDensity.clear();

    double dDeltaLonRad    = m_pQMRBiomeBase->getDLon()*M_PI/180; printf("DeltaLon: %f\n", dDeltaLonRad);
    double dDeltaLatRad    = m_pQMRBiomeBase->getDLat()*M_PI/180; printf("DeltaLat: %f\n", dDeltaLonRad);
    double dDeltaLatRad2   = m_pQMRBiomeBase->getDLat()*M_PI/360; // half of delta lat;

    //   m_mapBiomePercentages = mapPercentages;
    for (unsigned int i = 0; (iResult == 0) && (i < vSpecies.size()); i++) {
        // does the species exist in the percentage list?
        const char *pSpecies = vSpecies[i].c_str();
        if (m_pPR->hasSpecies(pSpecies)) {
            m_mDensity[vSpecies[i].c_str()] = dNaN;
        } else {
            printf("Species [%s] is not in the percentage list\n", pSpecies);
            iResult = -2;
        }

    }

    if (iResult == 0) {

        unsigned int iNumSpecies = vSpecies.size();
        if (mTotalMass.size() == iNumSpecies) {
            printf("calculating base mass\n");
            if (m_pQMRBiomeBase != NULL) {
                int iW = m_pQMRBiomeBase->getNLon();
                int iH = m_pQMRBiomeBase->getNLat();
                printf("biome data: %dx%d\n", iW, iH);

                bool bOK = m_pQMRBiomeBase->extractData();
                if (bOK) {
                    uchar **aaucBiomesBase = m_pQMRBiomeBase->getData();

                    time_t t1 = clock();
                    double *adLon = new double[iW]; 
                    for (int iX = 0; iX < iW; ++iX) {
                        adLon[iX]  = m_pQMRBiomeBase->X2Lon(iX);
                    }

                    VEC_DOUBLES vSumReal(iNumSpecies, 0);
                    double dTotalArea = 0;
                    double dTotalArea2 = 0;
                    double dLandArea  = 0;
#ifdef OMP
#pragma omp parallel for
#endif
                    for (int iY = 0; iY < iH; ++iY) {
                        double dLat = m_pQMRBiomeBase->Y2Lat(iY); // need latitude in degrees for func
                        double dLatRad = dLat*M_PI/180;
                        double dC2 = cos(dLatRad);
                        VEC_DOUBLES vSumRealLat(iNumSpecies, 0);
                        double ddTA = 0;
                        double ddTA2 = 0;
                        double ddLA = 0;
                        // these values are needed to calculate the area of a cell:
                        // A = R^2 *|sin(Lat1)-sin(Lat2)|*|Lon1 - Lon2| 
                        //   = R^2 *|sin(Lat1)-sin(Lat2)|* deltaLon (everything in radians)
                        double dPhi1 = dLatRad - dDeltaLatRad2;
                        double dPhi2 = dLatRad + dDeltaLatRad2;
                        double dDeltaSinLat = fabs(sin(dPhi1) - sin(dPhi2));

                        for (int iX = 0; iX < iW; ++iX) {
                            uchar ucB    = aaucBiomesBase[iY][iX];
                            double dLocFactor = (*m_pfuncLocDep)(adLon[iX], dLat);
                            for (unsigned int k = 0; k < iNumSpecies; k++) {
                                vSumRealLat[k] += m_pPR->getPercentageAbs(vSpecies[k].c_str(), ucB)*dLocFactor;
                            }
                            // debug: total area
                            ddTA += 1;
                            
                            // double check grass density with different method (cos)
                            ddTA2 += m_pPR->getPercentageAbs(vSpecies[0].c_str(), ucB)*dLocFactor;

                            // land area                            }
                            ddLA += ((ucB!=VAL_OCEAN)&&(ucB!=VAL_UNDEF))?1:0;
                        }
                        for (unsigned int k = 0; k < iNumSpecies; k++) {
                            vSumReal[k] += vSumRealLat[k]*dDeltaSinLat;
                        }
                        dTotalArea  += ddTA  * dDeltaSinLat;
                        dTotalArea2 += ddTA2 * dC2;
                        dLandArea   += ddLA  * dDeltaSinLat;
                           printf("\rrow %03d:  %f", iY, vSumReal[0]); fflush(stdout);

                    }
                    printf("\n");
                
                    delete[] adLon;
                    dTotalArea  *= dDeltaLonRad;
                    dTotalArea2 *= dDeltaLonRad*dDeltaLatRad;
                    dLandArea   *= dDeltaLonRad;
                    printf("unit:check total area (sin): %f, total area grass: %f,  Ideal Sphere: %f, Land area %f\n", dTotalArea, dTotalArea2, 4*M_PI, dLandArea);
                    printf("real:check total area (sin): %e, total area grass: %e,  Ideal Sphere: %e, Land area %e\n", dTotalArea*RADIUS_EARTH2, dTotalArea2*RADIUS_EARTH2, 4*M_PI*RADIUS_EARTH2, dLandArea*RADIUS_EARTH2);
                    printf("Rad %f, rad2 %f\n", RADIUS_EARTH, RADIUS_EARTH2);
                    printf("  %s: orig area %e, alt area %e\n", vSpecies[0].c_str(), vSumReal[0]*dDeltaLonRad*RADIUS_EARTH2, dTotalArea2*RADIUS_EARTH2);
                    for (unsigned int k = 0; k < iNumSpecies; k++) {
                        vSumReal[k] *=dDeltaLonRad*RADIUS_EARTH2;
                        if (vSumReal[k] > 0) {
                            m_mDensity[vSpecies[k]] = mTotalMass[vSpecies[k]]/vSumReal[k];
                            printf("[%s]Real:\tTot:%e, sum:%e -> dens:%e\n", vSpecies[k].c_str(), mTotalMass[vSpecies[k]], vSumReal[k], m_mDensity[vSpecies[k]]);
                        } else {
                            m_mDensity[vSpecies[k]] = 0;
                        }
                    }
                    t1 = clock() - t1;
                    printf("Time: %.f\n", t1*1.0);
                
                } else {
                    printf("Couldn't extract data from BiomeFile\n");
                }
            } else {
                printf("Couldn't open BiomeFile\n");
            }
        } else {
            printf("Totals vector has not the same size as species vector\n");
        }
    } else {
        if (iResult == -2) {
            printf("Species listed in biome file: (");
            m_pPR->displaySpecies(true);
            printf(")\n");
        } else {
            printf("no function and/or Percentage file has been specified\n");
        }
    }

   return iResult;
}



//------------------------------------------------------------------------------
// calcMass
//
int BiomassDist::calcMass(char *pBiomeTargetFile, const char *pSpecies, char *pOutput, int iType) {
    int iResult = -1;
    double dSecondSum = 0;
    int iMapType;

    QMapReader<uchar> *pQMRBiomeTarget = dynamic_cast<QMapReader<uchar>*>(QMapUtils::createValReader(pBiomeTargetFile, false, &iMapType));
    if (pQMRBiomeTarget != NULL) {
        double dDeltaLonRad    = pQMRBiomeTarget->getDLon()*M_PI/180; 
        double dDeltaLatRad2   = pQMRBiomeTarget->getDLat()*M_PI/360; // half of delta lat;
        
        iResult = 0;
        // createValReader() already extracted data
        uchar **aaucBiomesTarget = pQMRBiomeTarget->getData();
        int iWT = pQMRBiomeTarget->getNLon();
        int iHT = pQMRBiomeTarget->getNLat();

        // save the longitude values we need several times
        double *adLon = new double[iWT]; 
        for (int iX = 0; iX < iWT; ++iX) {
            adLon[iX]  = pQMRBiomeTarget->X2Lon(iX);
        }
        double dArea = 0;
        // create and fill array of productivity for plant
        double **aadMass = new double*[iHT];
#ifdef OMP
#pragma omp parallel for
#endif
        for (int iY = 0; iY < iHT; ++iY) {
            double dLat = pQMRBiomeTarget->Y2Lat(iY);
            double dLatRad = dLat*M_PI/180;
            // these values are needed to calculate the area of a cell:
            // A = R^2 *|sin(Lat1)-sin(Lat2)|*|Lon1 - Lon2| 
            //   = R^2 *|sin(Lat1)-sin(Lat2)|* deltaLon (everything in radians 
            double dPhi1 = dLatRad - dDeltaLatRad2;
            double dPhi2 = dLatRad + dDeltaLatRad2;
            double dDeltaSinLat = fabs(sin(dPhi1) - sin(dPhi2));
            
            aadMass[iY] = new double[iWT];
            for (int iX = 0; iX < iWT; ++iX) {
                uchar ucB    = aaucBiomesTarget[iY][iX]; 
                if ((ucB != VAL_OCEAN) && (ucB != VAL_UNDEF)) {
                    // or perhaps: dDeltaSinLat*dDeltaLonRad*dV*m_mapBiomePercentages[ucB]*m_dFactor
                    aadMass[iY][iX] = m_mDensity[pSpecies] * m_pPR->getPercentageAbs(pSpecies, ucB) * (*m_pfuncLocDep)(adLon[iX], dLat);
                    if (iType == TYPE_MASS) {
                        aadMass[iY][iX] *=  dDeltaSinLat*dDeltaLonRad*RADIUS_EARTH2;
                    }
                    dSecondSum +=  aadMass[iY][iX];
                    dArea += m_pPR->getPercentageAbs(pSpecies, ucB)*dDeltaSinLat*dDeltaLonRad*RADIUS_EARTH2;
                } else {
                    aadMass[iY][iX] = dNaN;
                }
                /*
                if ((iY >= 438) && (iY <= 440) && (iX >= 339) && (iX <= 343)) {
                    printf("(%s) At (%d,%d)[%e(%e)]: M:%e A:%e\n", pSpecies, iX, iY, 
                           dDeltaSinLat*dDeltaLonRad*RADIUS_EARTH2, 
                           cos(dLatRad)*dDeltaLonRad*RADIUS_EARTH2*dDeltaLatRad2*2,
                           aadMass[iY][iX],  
                           m_pPR->getPercentageAbs(pSpecies, ucB)*dDeltaSinLat*dDeltaLonRad*RADIUS_EARTH2);
                } 
                */
            }
        }
        
        delete[] adLon;
        
        printf("Control for %-5s: total mass: %e, area: %e -> dens: %e\n", pSpecies, dSecondSum, dArea, dSecondSum/dArea);
        
        // now create output
        m_pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                pQMRBiomeTarget->getLonMin(), pQMRBiomeTarget->getLonMax(), pQMRBiomeTarget->getDLon(),  
                                pQMRBiomeTarget->getLatMin(), pQMRBiomeTarget->getLatMax(), pQMRBiomeTarget->getDLat());
        
        // create file
        FILE *fOut = fopen(pOutput, "wb");
        m_pQMH->addHeader(fOut);
        
        // fill data
        for (int iY = 0; iY < iHT; ++iY) {
            fwrite(aadMass[iY], sizeof(double), iWT, fOut);
        }
        // close file
        fclose(fOut);
        
        // free data
        if (aadMass != NULL) {
            for (int i = 0; i < iHT; ++i) {
                if (aadMass[i] != NULL) {
                    delete[] aadMass[i];
                }
            }
            delete[] aadMass;
        }
    } else {
        printf("Couldn't open Reader for [%s] (wrong  - Type should be %d (%s), was %d (%s)\n", pBiomeTargetFile, QMAP_TYPE_UCHAR, QMapHeader::getTypeName(QMAP_TYPE_UCHAR), iMapType, QMapHeader::getTypeName(iMapType));
    }
    
    return iResult;
}
    
//------------------------------------------------------------------------------
// calcGlobalDensity
//   Calculate base mass value whose dependence is given by funcLocDep for a
//   plant whose area coverage for a given biome value is given by a simple
//   text file consisting of line of the form
//      <biome_value> <percentage>
//
int BiomassDist::calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass, VEC_STRINGS &vSpecies, TrinaryFunc *pfuncLocDep, char *pPercentageFile) {
    int iResult = -1;
    setParams(pfuncLocDep, pPercentageFile);
    if (m_bReady) { 
        //m_pPR->display();
        iResult = calcGlobalDensity(mTotalMass, vSpecies);
    } else {
        printf("Bad percentage file?\n");
    }
    return iResult;
}

//------------------------------------------------------------------------------
// calcGlobalDensity
//   Calculate base mass value whose dependence is given by funcLocDep for a
//   plant whose area coverage for a given biome value is given by a simple
//   text file consisting of line of the form
//      <biome_value> <percentage>
//
int BiomassDist::calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass, VEC_STRINGS &vSpecies, TrinaryFunc *pfuncLocDep, PercentageReader *pPR) {
    int iResult = -1;
    setParams(pfuncLocDep, pPR);
    if (m_bReady) { 
        //m_pPR->display();
        iResult = calcGlobalDensity(mTotalMass, vSpecies);
    } else {
        printf("Bad percentage file?\n");
    }
    return iResult;
}

//------------------------------------------------------------------------------
// calcBiomass
//   
double BiomassDist::calcBiomass(double dLon, double dLat) {
    
    if (dLat != m_dLastLat) {
        m_dLastLat = dLat;
        m_iLastY   = (int) m_pQMRBiomeBase->Lat2Y(dLat);
    }
    
    int iX = (int) m_pQMRBiomeBase->Lon2X(dLon);
  
    printf("(%f,%f) -> [%d,%d]:%f\n", dLon, dLat, iX, m_iLastY, m_aadMass[m_iLastY][iX]);
    return m_aadMass[m_iLastY][iX];
}

//------------------------------------------------------------------------------
// getBiomass
//   
double BiomassDist::getBiomass(int iX, int iY) {
    
    return m_aadMass[iY][iX];
}



