#include <stdlib.h>
#include <math.h>
#include "xrand.h"
#include "utils.h"
#include "strutils.h"
#include "ids.h"

#include <omp.h>

#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "Climate.h"
#include "Geography.h"
#include "NPPVeg.h"

#include "QMapHeader.h"
#include "SnapHeader.h"
#include "SCellGrid.h"

#define SQR3 1.732

static  unsigned int aulVegState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b,
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5,
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f,
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};


//-----------------------------------------------------------------------------
//  constructor
//
Vegetation::Vegetation(uint iNumCells, int iNumVegSpecies, Geography *pGeography, Climate *pClimate)
    : m_bUpdated(true),
      m_iNumCells(iNumCells),
      m_iNumVegSpecies(iNumVegSpecies),
      m_fPreviousTime(-1.0),
      m_pGeography(pGeography),
      m_pClimate(pClimate) {

    m_adMass = createArray();
    m_adANPP = createArray();
    
    m_abDynamic = new bool[m_iNumVegSpecies];
    for (int i=0; i<m_iNumVegSpecies; i++) {
        m_abDynamic[i] = false;
    }

    m_adVariance = new double[m_iNumVegSpecies];
    for (int i=0; i<m_iNumVegSpecies; i++) {
        m_adVariance[i] = 0;
    }
    

    int iNThreads = omp_get_max_threads();
    m_apWELL = new WELL512*[iNThreads];
#ifdef OMP_A
#pragma omp parallel 
    {	
#endif
        int iT = omp_get_thread_num();
        unsigned int temp[STATE_SIZE];
        for (unsigned int j = 0; j < STATE_SIZE; j++) {
            temp[j] = aulVegState[(iT+13*j)%16];
        }
        m_apWELL[iT] = new WELL512(temp);	
#ifdef OMP_A
    }
#endif
  
}



//-----------------------------------------------------------------------------
//  destructor
//
Vegetation::~Vegetation() {
    deleteArray(m_adMass);
    deleteArray(m_adANPP);
    delete[] m_abDynamic;

    for (int i = 0; i < omp_get_num_threads(); i++) {
        delete m_apWELL[i];
    }
    delete[] m_apWELL;

}


//-----------------------------------------------------------------------------
//  createArray
//
double **Vegetation::createArray() {
    double **adData = new double* [m_iNumVegSpecies];
    for (int i = 0; i < m_iNumVegSpecies; i++) {
        adData[i] = new double[m_iNumCells];
        if (adData[i] != NULL)  {
            for (uint j = 0; j < m_iNumCells; j++) {
                adData[i][j] = dNaN;
            }
        } else {
            fprintf(stderr,"Vegetation could not create array\n");
        }
    }
    return adData;
}

//-----------------------------------------------------------------------------
//  deleteArray
//
void Vegetation::deleteArray(double **adData) {
    // free adData
    if (adData != NULL) {
        for (int i = 0; i < m_iNumVegSpecies; i++) {
            if (adData[i] != NULL) {
                delete[] adData[i];
            }
        }
        delete[] adData;
    }
} 



//-----------------------------------------------------------------------------
//  updateANPP
//
void Vegetation::updateClimateANPP(int iSpecies) {

    if (m_abDynamic[iSpecies]) {
        
        // here the ANPP calculations are done with
        // formulas for the MEAN ANNUAL values.
        // I hope it's not too bad to use them as
        // approximations of "instantaneous" ANPP
        
        // we can include variance in NPP (which will result in "patchyness")
        // determined stochastically for each calculation of NPP
        
        // we will distinguish between tree and not-tree (Del Grosso et al. 2008)
        
        // conversions are:
        // 0.4 kg C = 1 kg Dry Matter
        // 1 kg Dry Matter = 18.5 MJ
        // see Erb et al 09, Cebrian 99, GCEP Energy Assessment Analysis 2005

        // ADDED MY OWN TEMPERATURE MODULATION OF NPP
        // BECAUSE DEL GROSS DOES NOT CONSIDER IT
        // see e.g. Admans White & Lenton 2004 
        
        if(iSpecies == SPC_GRASS) { 
            
#ifdef OMP_A
#pragma omp parallel for
#endif
            for (uint i = 0; i < m_iNumCells; i++) {
                
                // with temperature correction
                if (m_pGeography->m_adAltitude[i] >= 0 && m_pClimate->m_adAnnualMeanTemp[i] > 0) {
                    climatenumber dP = m_pClimate->m_adActualRains[i];
                    
    	            m_adANPP[iSpecies][i] = 10000 * (1 - exp(-4.77e-5 * dP)); // g m^-2 yr^-2


                    if (m_adVariance[iSpecies] > 0) {
                        // ADD DISPERSION, RMSE ~ 90 in DelGrosso
                    
                        // using Marsaglia's polar method to generate Gaussian variable
                        int iThread = omp_get_thread_num();
                    
                        double dU = 1;
                        double dV = 1;
                        double dS = dU * dU + dV * dV;
                        while ( dS >= 1 ) {			
                            dU = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dV = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dS = dU * dU + dV * dV;
                        }
                        double dX = dU * sqrt(-2.*log(dS)/dS);
                    
                        // m_adANPP[iSpecies][i] += dX * 90; 

                        // see variance of uniform distribution
                        m_adANPP[iSpecies][i] *= (1.0 + SQR3 * m_adVariance[iSpecies] * dX);
                    }                    
                    
                    // my own correction, inspired by Lenton
                    if (m_adANPP[iSpecies][i] < 0.0) {
                        m_adANPP[iSpecies][i] = 0.0;
                    } else {
                        double dT = m_pClimate->m_adActualTemps[i];
                        m_adANPP[iSpecies][i] *= dT * dT * (40. - dT) / 9500.;
                    }
                    
                } else {
                    m_adANPP[iSpecies][i] = 0.0;
                }
            }
            
        } else if(iSpecies == SPC_BUSH) { 
            
#ifdef OMP_A
#pragma omp parallel for
#endif
            for (uint i = 0; i < m_iNumCells; i++) {
                
                // with temperature correction
                if (m_pGeography->m_adAltitude[i] >= 0 && m_pClimate->m_adAnnualMeanTemp[i] > 0) {
                    climatenumber dP = m_pClimate->m_adActualRains[i];
                    
    	            m_adANPP[iSpecies][i] = 10000 * (1 - exp(-4.77e-5 * dP)); // g m^-2 yr^-2

                    
                    if (m_adVariance[iSpecies] > 0) {
                        // ADD DISPERSION, RMSE ~ 90 in DelGrosso
                        // using Marsaglia's polar method to generate Gaussian variable
                        int iThread = omp_get_thread_num();
                    
                        double dU = 1;
                        double dV = 1;
                        double dS = dU * dU + dV * dV;
                        while ( dS >= 1 ) {			
                            dU = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dV = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dS = dU * dU + dV * dV;
                        }
                        double dX = dU * sqrt(-2.*log(dS)/dS);
                    
                        // m_adANPP[iSpecies][i] += dX * 45; 
                        m_adANPP[iSpecies][i] *= (1.0 + SQR3 * m_adVariance[iSpecies] * dX);
                    }
                        
                    // my own correction, inspired by Lenton
                    if (m_adANPP[iSpecies][i] < 0.0) {
                        m_adANPP[iSpecies][i] = 0.0;
                    } else {
                        double dT = m_pClimate->m_adActualTemps[i];
                        m_adANPP[iSpecies][i] *= dT * dT * (40. - dT) / 9500.;
                    }
                    
                } else {
                    m_adANPP[iSpecies][i] = 0.0;
                }
            }
            
        } else {

#ifdef OMP_A
#pragma omp parallel for
#endif             
            for (uint i = 0; i < m_iNumCells; i++) {
		
                if (m_pGeography->m_adAltitude[i] >= 0 && m_pClimate->m_adAnnualMeanTemp[i] > -10) {
                    climatenumber dP = m_pClimate->m_adActualRains[i];
                    climatenumber dT = m_pClimate->m_adActualTemps[i];
                    
                    if (dP > 0) {
                        m_adANPP[iSpecies][i] = 0.41625 * pow(dP,1.185) / exp(0.000414 * dP);  // g m^-2 yr^-2
                    } else {
                        m_adANPP[iSpecies][i] = 0.0;
                    } 
                    double dANPP_T = 7847.5 / (1 + exp(2.2 - 0.0307 * dT));  // g m^-2 yr^-2
                     
    	            if (dANPP_T < m_adANPP[iSpecies][i]) {
                        m_adANPP[iSpecies][i] = dANPP_T;
            	    }
                    
                    
                    if (m_adVariance[iSpecies] > 0) {
                        // ADD DISPERSION, RMSE ~ 200 for trees in DelGrosso
                        // using Marsaglia's polar method to generate Gaussian variable
                        int iThread = omp_get_thread_num();
                    
                        double dU = 1;
                        double dV = 1;
                        double dS = dU * dU + dV * dV;
                        while ( dS >= 1 ) {			
                            dU = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dV = 1. - 2. * m_apWELL[iThread]->wrandd();
                            dS = dU * dU + dV * dV;
                        }
                        double dX = dU * sqrt(-2.*log(dS)/dS);
                    
                        // m_adANPP[iSpecies][i] += dX * 100;
                        m_adANPP[iSpecies][i] *= (1.0 + SQR3 * m_adVariance[iSpecies] * dX);
                    }
                    
                    
                    if (m_adANPP[iSpecies][i] < 0.0) {
                        m_adANPP[iSpecies][i] = 0.0;
                    }
            
                } else {
                    m_adANPP[iSpecies][i] = 0.0;
                }
            }
        }
    } 
}




//-----------------------------------------------------------------------------
//  computeDetritusP
//
double Vegetation::computeDetritusP(double dANPP) {
    double dDet = 0;

    // from Cebrian 1999, Table 1 and Figure 7

    dDet = 0.63 * pow(dANPP, 1.1);   // check units

    return dDet;
}



//-----------------------------------------------------------------------------
//  update
//
int Vegetation::update(float fTime) {
    int iResult = 0;
    
    if (m_pClimate->m_bUpdated) {
		printf("[Vegetation::update] t = %f\n", fTime);
    	climateUpdate(fTime);
		m_bUpdated = true;
    }

    if (fTime > m_fPreviousTime) {

        for (int iSpecies = 0; iSpecies < m_iNumVegSpecies; iSpecies++) {
            printf("%d ",m_abDynamic[iSpecies]);
            if(m_abDynamic[iSpecies]) {
                // growth
                /*
                  #ifdef OMP_A
                  #pragma omp parallel for
                  #endif
                  for (uint i = 0; i < m_iNumCells; i++) {
                  if (m_pGeography->m_adAltitude[i] >= 0) {
                  m_adMass[iSpecies][i] += m_adANPP[iSpecies][i];  // HERE AREA NEEDED ! ! ! 
                        m_adMass[iSpecies][i] -= computeDetritusP(m_adANPP[iSpecies][i]);
                        } else {
                        m_adMass[iSpecies][i] = 0.0;
                        }
                        }
                */
            }
        }
        m_fPreviousTime = fTime;
    } 
    printf("\n");
    
    return iResult;
}

//-----------------------------------------------------------------------------
//  update
//
int Vegetation::climateUpdate(float fTime) {
    int iResult = 0;
    
        
    for (int iSpecies = 0; iSpecies < m_iNumVegSpecies; iSpecies++) {
        if(m_abDynamic[iSpecies]) {
            updateClimateANPP(iSpecies);
        }
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
//  outputVeg
//  writes snap or qmap of altitudes
//
void Vegetation::writeOutput(float fTime, int iStep, SCellGrid* pCG) {

    if (m_pGeography->m_dRadius == 0) {  // if grid is flat, output QMAP directly
        writeVegQMap(fTime, iStep);
    } else {  // if grid is ico, output SNAP to view with IQApp
        writeVegSnap(fTime, iStep, pCG);
    }
    
}


//-----------------------------------------------------------------------------
//  writeVegQMap
//
void Vegetation::writeVegQMap(float fTime, int iStep) {
    
    for (int i=0; i<m_iNumVegSpecies; i++) {
        
        char sColName[128];
        sprintf(sColName, "Veg%d",i);

        QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                          m_pGeography->m_adQMapHeadData,
                                          sColName, "X", "Y"); 
        
        char* sOutFile = new char[128]; 
        sprintf(sOutFile,"veg%d_%05d.qmap",i,iStep);
        
        FILE *fOut = fopen(sOutFile, "wb");
        
        if (fOut != NULL) {
            
            pQMH->addHeader(fOut);
            fwrite(m_adMass[i], sizeof(double), m_iNumCells, fOut);
            fclose(fOut);
            
        } else {
            fprintf(stderr,"writing file %s failed\n",sOutFile);
        }
    }
}
      
//-----------------------------------------------------------------------------
// writeSnapVeg
//
void Vegetation::writeVegSnap(float fTime, int iStep, SCellGrid* pCG) {
    SnapHeader *pSH = new SnapHeader("3.0", COORD_NODE, iStep, fTime, "ld", "dummy.ico",false, 7776, "Veg",0,NULL);
    char sOut[128];
    sprintf(sOut, "vegtotal_%05d.snap", iStep);
    FILE *fOut = fopen(sOut, "wb");
    pSH->write(fOut, true);

    unsigned char *pBuffer = new unsigned char[m_iNumCells*(sizeof(gridtype)+sizeof(double))];
    unsigned char *p = pBuffer;
    for (uint i = 0; i < m_iNumCells; i++) {
        p = putMem(p, &(pCG->m_aCells[i].m_iGlobalID), sizeof(gridtype));
        double dSum = 0;
        for (int iSpecies =0; iSpecies < m_iNumVegSpecies; iSpecies++) {
            dSum += m_adMass[iSpecies][i];
        }
        p = putMem(p, &dSum, sizeof(double));
    }

    fwrite(pBuffer, sizeof(gridtype)+sizeof(double), m_iNumCells, fOut);

    fclose(fOut);

}
