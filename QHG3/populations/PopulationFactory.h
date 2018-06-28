#ifndef __POPULATIONFACTORY_H__
#define __POPULATIONFACTORY_H__

#include <stdint.h>

class SCellGrid;
class LineReader;
class PopBase;
class PopFinder;
class IDGen;

class PopulationFactory {
public:
    PopulationFactory(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);

    PopBase *readPopulation(const char *pConfig);
    PopBase *createPopulation(spcid iClassID);
    PopBase *createPopulation(const char *pClassName);

protected:
    SCellGrid *m_pCG;
    PopFinder *m_pPopFinder;
    int        m_iLayerSize;
    IDGen    **m_apIDG;
    uint32_t  *m_aulState;
    uint      *m_aiSeeds;

    static spcid readClass(LineReader *pLR);
};

#endif
