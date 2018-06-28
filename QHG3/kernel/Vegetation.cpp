#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"
#include "strutils.h"
#include "ids.h"

#include <omp.h>

#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "Climate.h"
#include "Geography.h"
#include "Vegetation.h"

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
    : m_iNumCells(iNumCells),
      m_iNumVegSpecies(iNumVegSpecies),
      m_fPreviousTime(-1.0),
      m_pGeography(pGeography),
      m_pClimate(pClimate) {

    m_adBaseANPP = new double[m_iNumCells];
    m_adTotalANPP = new double[m_iNumCells];
    memset(m_adBaseANPP, 0, m_iNumCells*sizeof(double));
    memset(m_adTotalANPP, 0, m_iNumCells*sizeof(double));
    

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
    delete[] m_adBaseANPP;
    delete[] m_adTotalANPP;

    for (int i = 0; i < omp_get_num_threads(); i++) {
        delete m_apWELL[i];
    }
    delete[] m_apWELL;

}


//-----------------------------------------------------------------------------
//  update
//
int Vegetation::update(float fTime) {
    int iResult = 0;
   
    climateUpdate(fTime);
    m_fPreviousTime = fTime;
    
    // nothing else to do 
    
    return iResult;
}


//-----------------------------------------------------------------------------
//  climateUpdate
//
int Vegetation::climateUpdate(float fTime) {
    int iResult = 0;
    
    // nothing to do
    
    return iResult;
}


