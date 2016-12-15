#ifndef __ALTMOVERFKPOP_H__
#define __ALTMOVERFKPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "Verhulst.h"

struct AltMoverFKAgent : Agent {

};

// int *iPtr; a pointer to an integer value
// SCellGrid *pCG   pointer to the value of type SCellGrid
class AltMoverFKPop : public SPopulation<AltMoverFKAgent> {
 
 public:
   AltMoverFKPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
   // uint32_t - typedef unsigned long int
   ~AltMoverFKPop();
 
 protected:
    SingleEvaluator<AltMoverFKAgent> *m_pSE;
    WeightedMove<AltMoverFKAgent> *m_pWM;  
    double *m_adEnvWeights; // pointer to an array of [Ncells*(Nneighbors +1)]; where 
   							// the relative statistical weights of each cell and its
                            // neighbors are stored
    Verhulst<AltMoverFKAgent> *m_pVer; 		
};

#endif 
