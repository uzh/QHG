#ifndef __PDHUNTING_H__
#define __PDHUNTING_H__

#include "Action.h"
#include "PreyDistributor.h"

#define PDHUNTING_NAME           "PDHunting"
#define PDHUNTING_RELATIONS_NAME "PDHunting_relations"

#define NAME_LEN 1024

typedef std::pair<double,double>          doublepair;
typedef std::map<std::string, doublepair> relation;

class MassInterface;
class PopFinder;


template<typename T>
class PDHunting : public Action<T> {
public:
    PDHunting(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, PopFinder *pPopFinder);
    virtual ~PDHunting();

    int preLoop();
    int initialize(float fTime);
    int operator()(int iA, float fT);
    int postLoop();
    
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);


protected:
    int relationFromString(char *pRelations);

    WELL512 **m_apWELL;
    double m_dEfficiency;
    double m_dUsability;
    std::map<std::string, MassInterface *> m_mpMIPrey;
    MassInterface *m_pMIPred;
    PopFinder *m_pPopFinder;
    // maybe unneeded
    std::map<std::string, PopBase *> m_mpPreyPop;

    relation   m_mRelations;
    preyratio *m_pPreyEff;
    std::string m_sPredName;

    char m_sRelationInput[NAME_LEN];

};


#endif
