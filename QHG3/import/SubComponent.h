#ifndef __SUBCOMPONENT_H__
#define __SUBCOMPONENT_H__

#include <map>
#include <vector>

#include "types.h"

typedef std::map<gridtype, double>   distlist;
typedef std::map<gridtype, distlist> distancemap;

typedef std::set<gridtype>        cellchain;

class SubComponent {
public:
    SubComponent(int iID);
    ~SubComponent();

    int getID() { return m_iID;};

    void setCells(cellchain vCellChain);
    cellchain &getChain() { return m_vCellChain;};
protected:
    int       m_iID;
    cellchain m_vCellChain;

};


#endif
