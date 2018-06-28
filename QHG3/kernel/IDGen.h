#ifndef __IDGEN_H__
#define __IDGEN_H__
/****************************************************************************
 * IDGen is used for globally unique IDs
 *   iB     is the base, i.e. the lowest ID to be created
 *   iOffs  is an offset to distinguish IDs of different threads;
 *          usually the number of the thread (for multi node apps a global thread number)
 *   iStep  increment of two consecutive IDs of a thread;
 *          typically the total number of threads
 ****************************************************************************/
 
#include <omp.h>
#include "types.h"

class IDGen {
public:
    IDGen(idtype iB, idtype iOffs, idtype iStep) 
        : m_iOffs(iOffs),
          m_iStep(iStep),
          m_iCur(0) {

        setBase(iB);
    }

    void setBase(idtype iB) { m_iCur = iB + m_iOffs - m_iStep; };
    void setData(idtype iB, idtype iOffs, idtype iStep) { m_iOffs=iOffs; m_iStep=iStep; setBase(iB);};

    inline idtype getID() { return m_iCur+= m_iStep; };

    // dump&restore
    idtype getCur() { return m_iCur;};
    void   setCur(idtype iCur) { m_iCur = iCur;};
private:
    idtype m_iOffs;
    idtype m_iStep;
    idtype m_iCur;
};

#endif
