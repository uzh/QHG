#ifndef __PRIORITIZER_H__
#define __PRIORITIZER_H__

#include <map>
#include <set>
#include <vector>
#include <string>
#include <hdf5.h>

#include "types.h"
#include "SPopulation.h"

template <typename A> class Action;

template <typename A>
class Prioritizer {
public:
    typedef std::map<uint, std::vector< Action<A>* > > methlist;
    typedef std::map<std::string, Action<A>* > namelist;

    Prioritizer() {};
    // virtual destructor for nice clean up
    virtual ~Prioritizer() {};

    // associate priority values with methods
    virtual void setPrio(uint iPrio, std::string name);

    // associate names with actions
    virtual void addAction(std::string name, Action<A> *act);

    // remove an action completely (can't be undone - must call addAction() again)
    virtual int removeAction(std::string name);

    // return highest priority value
    virtual uint getMaxPrio() { return m_prios.rbegin()->first;};

    // return highest priority value
    virtual uint getPrios( std::set<uint> &vPrios);

    uint getNumMethodsForPrio(int iPrio) { return (uint)m_prios[iPrio].size();};

    Action<A> *getMethod(int iPrio, int iWhich) {return m_prios[iPrio][iWhich];};

    int extractActionParamsQDF(hid_t hSpeciesGroup); // for qdf
    int writeActionParamsQDF(hid_t hSpeciesGroup); // for qdf
    int readActionParamLine(char *pLine); // for ascii pop

    void showAttributes();
protected:
    namelist m_names;
    methlist m_prios;
};

#endif
