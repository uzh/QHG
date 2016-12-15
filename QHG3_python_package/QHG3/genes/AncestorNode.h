#ifndef __ANCESTORNODE_H__
#define __ANCESTORNODE_H__

#include <set>

#include "types.h"

const int MOM = 0;
const int DAD = 1;

//----------------------------------------------------------------------------
// AncestorNode
//   
class AncestorNode {
public:
    AncestorNode(idtype iID, idtype iMomID=-1, idtype iDadID=-1)
        : m_iID(iID),
          m_bSelected(false),
          m_iGender(-1) {
        m_aParents[MOM] = iMomID;
        m_aParents[DAD] = iDadID;
    };


    int  merge(AncestorNode *pAN);
    void setMom(idtype iMomID) { m_aParents[MOM] = iMomID;};
    void setDad(idtype iDadID) { m_aParents[DAD] = iDadID;};
    void addChild(idtype iChildID) { m_sChildren.insert(iChildID);};

    idtype getMom() { return m_aParents[MOM];};
    idtype getDad() { return m_aParents[DAD];};
    idtype m_iID;
    idtype m_aParents[2];
    idset  m_sChildren;
    bool   m_bSelected;
    int    m_iGender;
};


#endif

