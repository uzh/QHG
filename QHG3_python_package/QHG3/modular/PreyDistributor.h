#ifndef __PREYDISTRIBUTOR_H__
#define __PREYDISTRIBUTOR_H__

#include <omp.h>
#include <vector>
#include <map>

#include "types.h"
#include "Action.h"

typedef std::vector<int>   intlist;
typedef std::pair<int,int> intpair;

// for each prey species a vector of (predID,preyID) pairs 
//typedef std::map<std::string, std::vector<intpair> > assignmentmap;
// for each prey species a vector of (predID,preyID) pairs 
typedef std::map<std::string, std::set<intpair> > assignmentmap;

// for each prey agentID a pair of (preyname, vector of (preyID) ) 
typedef std::map<idtype, std::pair<std::string, std::vector<idtype> > > assignmentmap2;

// vector of (preyspecies, hunt-efficiency) pairs
typedef std::vector<std::pair<std::string, float> >  relationvec;

// pred-prey relation element (predator name, hunt-efficiency)
// for exchange via ArrayShare
typedef  std::pair<std::string, float> preyratio;


typedef std::map<std::string, bool> flagmap;

#define PD_TEMPLATE_PREY    "%s_prey"
#define PD_TEMPLATE_INDEXES "%s_indexes"
#define PD_TEMPLATE_ASSMAP  "%s_ass"

class PreyDistributor {
public:
    PreyDistributor(int iNumCells, WELL512 **apWELL);
    virtual ~PreyDistributor();

    // in preloop:
    int registerPredator(const char *pPredName);

    // in initializeStep
    int buildAssignments(const char *pPredName, float fTime);

    // mainly debugging
    void showRelations();
    void showFrequencies();
    void showAssignments();
    void showAgentAssignments(int iCellID, const char *pPredName, int iAgentIndex);
    /*    void showUnmatched();*/
protected:
    int getFrequencies();
    int calcAssignments();

    // global
    // preyname => vector{predname_i, efficiency_i}
    std::map<std::string, relationvec>      m_mRelations;

    // cellwise
    // preyname => vector{ sum_k^i numpred_k}_i (cumulated numbers of preds in cell)
    std::map<std::string, intlist>         *m_avNum;

    // predator-wise: every predator gets an array of assignmentmaps
    // predname => array[cell]{ preyname => vector{(pred_ID, prey_ID)}
    std::map<std::string, assignmentmap* >  m_Ass;

    /*
    //@@ not really necessary: structs for statistics
    std::map<std::string, intset>          *m_amUnmatchedBadHunting;
    std::map<std::string, intset>          *m_amUnmatchedNoHunting;
    */

    int m_iNumCells;
    WELL512 **m_apWELL;
    int m_iNumThreads;

    float m_fLastTime;
    flagmap m_mbReady;
public:
    static PreyDistributor *createInstance(int iNumCells, WELL512 **apWELL);
    static PreyDistributor *getInstance();
    static void freeInstance();
protected:
    static PreyDistributor *s_pPD;

};

#endif
