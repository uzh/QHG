#ifndef __CONFINEDMOVE_H__
#define __CONFINEDMOVE_H__

#include "Action.h"

#define CONFINEDMOVE_NAME "ConfinedMove"
#define CONFINEDMOVE_X_NAME "ConfinedMoveX"
#define CONFINEDMOVE_Y_NAME "ConfinedMoveY"
#define CONFINEDMOVE_R_NAME "ConfinedMoveR"

class WELL512;

template<typename T>
class ConfinedMove : public Action<T> {
    
 public:
    ConfinedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::vector<int>** vMoveList);
    ~ConfinedMove();

    int initialize(float fTime);

    int finalize(float fTime);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

 protected:
    double m_dX;
    double m_dY;
    double m_dR;
    std::vector<int>** m_vMoveList;
    bool* m_bAllowed;
    bool m_bAbsorbing;
    bool m_bInitialized;
};

#endif
