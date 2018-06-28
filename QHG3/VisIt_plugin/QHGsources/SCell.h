#ifndef __SCELL_H__
#define __SCELL_H__

#define MAX_NEIGH 6

#include "types.h"

typedef int idtype;

typedef struct {
    idtype  m_iGlobalID;
    uchar   m_iNumNeighbors;
    int     m_aNeighbors[MAX_NEIGH];
} SCell;



#endif
