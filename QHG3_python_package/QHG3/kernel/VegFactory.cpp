#include <stdlib.h>
#include <string.h>
#include "ids.h"
#include "utils.h"
#include "strutils.h"
#include "NPPVeg.h"
#include "VegFactory.h"
#include "MessLogger.h"
#include "QMapUtils.h"
#include "ValReader.h"


//-----------------------------------------------------------------------------
// creator
//
VegFactory::VegFactory(const char *sDefFile, int iNumCells, Geography* pGeo, Climate* pClimate) 
    : m_iNumCells(iNumCells),
      m_pGeo(pGeo),
      m_pClimate(pClimate),
      m_pVeg(NULL) {
    
    m_pLR = LineReader_std::createInstance(sDefFile, "rt");

}


//-----------------------------------------------------------------------------
// destructor
//
VegFactory::~VegFactory() {

    // the VegFactory creates the Vegetation m_pVeg in readDef()
    // but never deletes it - it's returned by getVeg() and used elsewhere

    delete m_pLR;

}


//-----------------------------------------------------------------------------
// readDef
//
int VegFactory::readDef() {
    
    bool bInterpol;
    int iResult = 0;
    int iSpecies = -1;

    char sPar1[SHORT_INPUT];
    //    char sPar2[SHORT_INPUT];
    
    // if the grid is flat, do not interpolate:
    // lon e lat are used as true pixel coordinates
    if(m_pGeo->m_dRadius > 0) {
        bInterpol = true;
    } else {
        bInterpol = false;
    }

    // first, let's read how many veg species are in the def file

    char *pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
    pLine = trim(m_pLR->getCurLine());
    if (strstr(pLine, "NUM_VEG_SPC") == pLine) {
        
        if (sscanf(pLine, "NUM_VEG_SPC %s", sPar1) == 1) {

            int iNumVegSpc = atoi(sPar1);
            m_iNumVegSpecies = iNumVegSpc;

            // IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // here the Vegetation class is created
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            
            m_pVeg = new Vegetation(m_iNumCells, iNumVegSpc, m_pGeo, m_pClimate);
            
            // now that we have created the vegetation class,
            // we read and set the defs for each of the veg species

            pLine = m_pLR->getNextLine(GNL_IGNORE_ALL); 
            
            while ((iResult == 0) && !m_pLR->isEoF()) {
                pLine = trim(m_pLR->getCurLine());
                
                if (strstr(pLine, "SPECIES") == pLine) {
                    iNumVegSpc--;
                    
                    if(iNumVegSpc < 0) {
                        LOG_ERROR("[vegFactory::readDef] too many species in def file!\n");
                        iResult = -1;
                    } else {
                        
                        if(sscanf(pLine, "SPECIES %s", sPar1) == 1) {
                            int iT;

                            if (strToNum(sPar1, &iT)) {
                                iSpecies = iT;
                            } else {
                                iSpecies = spcValue(sPar1);  // as specified in "ids.h"
                            }

                            fprintf(stderr, "[VegFactory::readDef] Found veg species %i (expecting %i more)\n",iSpecies, iNumVegSpc);

                        } else {
                            fprintf(stderr, "[VegFactory::readDef] That's not a species I recognize: \"%s\"\n",pLine);
                            iResult = -1;
                        }
                    }
                    
                } else if (strstr(pLine, "DENS") == pLine && iSpecies > -1) {
                    if (sscanf(pLine, "DENS QMAP %s", sPar1) == 1 ||
                        sscanf(pLine, "DENSITY QMAP %s", sPar1) == 1) {
                        
                        ValReader *pVRDens = QMapUtils::createValReader(sPar1, bInterpol);
                        if(pVRDens == NULL) {
                            fprintf(stderr,"[VegFactory::readDef] could not create ValReader for file %s\n",sPar1);
                            iResult = -1;
                        } else {
                            setVegMassFromDensity(pVRDens, iSpecies);                        
                            delete pVRDens;
                        }
                    } else if (sscanf(pLine, "DENS %s", sPar1) == 1 ||
                               sscanf(pLine, "DENSITY %s", sPar1) == 1) {
                        
                        double dDens = strtod(sPar1, NULL);
                        setVegMassFromDensity(dDens, iSpecies);
                        
                    } else {
                        fprintf(stderr, "[VegFactory::readDef] don't understand %s\n",pLine);
                        iResult = -1;
                    }
                    
                } else if (strstr(pLine, "ANPP") == pLine && iSpecies > -1) {
                    if (sscanf(pLine, "ANPP QMAP %s", sPar1) == 1) {
                        
                        ValReader *pVRANPP = QMapUtils::createValReader(sPar1, bInterpol);
                        if(pVRANPP == NULL) {
                            fprintf(stderr,"[VegFactory::readDef] could not create ValReader for file %s\n",sPar1);
                            iResult = -1;
                        } else {
                            setDynamic(iSpecies, true);
                            setVegANPP(pVRANPP, iSpecies);                        
                            delete pVRANPP;
                        }
                    } else if (sscanf(pLine, "ANPP %s", sPar1) == 1) {
                        if (!strcmp(sPar1,"CLIMATE")) {
                            
                            setDynamic(iSpecies, true);
//                            iResult = m_pVeg->updateClimateANPP(iSpecies);
                            
                        } else if (!strcmp(sPar1,"NONE")) {
                            
                            setDynamic(iSpecies, false);
                            
                        } else {
                            
                            double dANPP = strtod(sPar1, NULL);
                            if(dANPP == 0.0) {
                                
                                setDynamic(iSpecies, false);
                                
                            } else {
                                
                                setDynamic(iSpecies, true);
                                setVegANPP(dANPP, iSpecies);
                            }
                        }

                    } else {
                        fprintf(stderr, "[VegFactory::readDef] don't understand %s\n",pLine);
                        iResult = -1;
                    }

                } else if (strstr(pLine, "END") == pLine) {
                    int iNumRead = sscanf(pLine, "SPECIES %s", sPar1);
                    if (iNumRead == 1) {
                        int iT;
                        if (!strToNum(sPar1, &iT)) {
                            iT = spcValue(sPar1);
                        }
                        if (iT != iSpecies) {
                            fprintf(stderr, "[VegFactory::readDef] I was expecting an end to species %i but got %s\n",iSpecies,pLine);
                            iResult = -1;
                        } else {
                            iSpecies = -1;  // reset species 
                        }
                    } 
                } else {
                    
                    fprintf(stderr, "[VegFactory::readDef] I don't understand %s\n",pLine);
                    iResult = -1;
                }
                
                pLine = m_pLR->getNextLine(GNL_IGNORE_ALL); 
            }
           
        } else {
            fprintf(stderr, "[VegFactory::readDef] please specify NUM_VEG_SPC in %s",pLine);
            iResult = -1;
        }
        
    } else {
        fprintf(stderr, "[VegFactory::readDef] please specify NUM_VEG_SPC first in the vegetation def file\n");
        iResult = -1;
    }
 
    if (iResult != 0) {

        if (m_pVeg != NULL) {
            delete m_pVeg;
        }
    }

    return iResult;
    
}

//-----------------------------------------------------------------------------
//  setVegMass
//
void VegFactory::setVegMassFromDensity(ValReader *pVRDens, int iSpecies) {

    if(m_pGeo->m_dRadius > 0) {
        for (int i = 0; i < m_iNumCells; i++) {
            double dLon = m_pGeo->m_adLongitude[i]*180/M_PI;
            double dLat = m_pGeo->m_adLatitude[i]*180/M_PI;
            double dArea = m_pGeo->m_adArea[i];
            m_pVeg->m_adMass[iSpecies][i] = dArea*pVRDens->getDValue(dLon, dLat);
        }
    } else { // flat grid
        for (int i = 0; i < m_iNumCells; i++) {
            unsigned int iX = (unsigned int)(m_pGeo->m_adLongitude[i]);
            unsigned int iY = (unsigned int)(m_pGeo->m_adLatitude[i]);
            double dArea = m_pGeo->m_adArea[i];
            m_pVeg->m_adMass[iSpecies][i] = dArea*pVRDens->getDValue(iX, iY);
        } 
    }

    return;
}

//-----------------------------------------------------------------------------
//  setVegMass
//
void VegFactory::setVegMassFromDensity(double dDens, int iSpecies) {

    for (int i = 0; i < m_iNumCells; i++) {
        double dArea = m_pGeo->m_adArea[i];
        m_pVeg->m_adMass[iSpecies][i] = dArea*dDens;
    }

    return;
}

//-----------------------------------------------------------------------------
//  setANPP
//
void VegFactory::setVegANPP(ValReader *pVRANPP, int iSpecies) {

    if(m_pGeo->m_dRadius > 0) {
        for (int i = 0; i < m_iNumCells; i++) {
            double dLon = m_pGeo->m_adLongitude[i]*180/M_PI;
            double dLat = m_pGeo->m_adLatitude[i]*180/M_PI;
            double dArea = m_pGeo->m_adArea[i];
            m_pVeg->m_adANPP[iSpecies][i] = dArea*pVRANPP->getDValue(dLon, dLat);
        }
    } else { // flat grid
        for (int i = 0; i < m_iNumCells; i++) {
            unsigned int iX = (unsigned int)(m_pGeo->m_adLongitude[i]);
            unsigned int iY = (unsigned int)(m_pGeo->m_adLatitude[i]);
            double dArea = m_pGeo->m_adArea[i];
            m_pVeg->m_adANPP[iSpecies][i] = dArea*pVRANPP->getDValue(iX, iY);
        }
    }

    return;
}

//-----------------------------------------------------------------------------
//  setANPP
//
void VegFactory::setVegANPP(double dANPP, int iSpecies) {

    for (int i = 0; i < m_iNumCells; i++) {
        double dArea = m_pGeo->m_adArea[i];
        m_pVeg->m_adANPP[iSpecies][i] = dArea*dANPP;
    }

    return;
}



//-----------------------------------------------------------------------------
//  setDynamic
//
void VegFactory::setDynamic(int iSpecies, bool bChoose) {

    if (iSpecies < m_iNumVegSpecies) {
        m_pVeg->m_abDynamic[iSpecies] = bChoose;
    } 

    return;
}


