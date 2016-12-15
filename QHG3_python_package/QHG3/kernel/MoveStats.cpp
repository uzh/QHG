#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "MoveStats.h"


MoveStats::MoveStats()
    : m_iNumCells(0),
      m_aiHops(NULL),
      m_adDist(NULL),
      m_adTime(NULL) {
}

MoveStats::MoveStats(uint iNumCells)
    : m_iNumCells(iNumCells),
      m_aiHops(NULL),
      m_adDist(NULL),
      m_adTime(NULL) {

    init(iNumCells);
}

int MoveStats::init(int iNumCells) {
        
    // make all hops -1
    m_aiHops = new int[m_iNumCells];
    memset(m_aiHops, 0xff, m_iNumCells*sizeof(uint));
    
    // make all dists 0
    m_adDist = new double[m_iNumCells];
    memset(m_adDist, 0x00, m_iNumCells*sizeof(double));
    
    // make all times 0
    m_adTime = new double[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        m_adTime[i] = -1;
    }
    //    memset(m_adTime, 0x00, m_iNumCells*sizeof(double));

#ifdef OMP_A
	m_aStatLocks = new omp_lock_t[m_iNumCells];
	for (uint i = 0; i < m_iNumCells; i++) {
		omp_init_lock(&m_aStatLocks[i]);
	}
#endif

    return 0;
}


MoveStats::~MoveStats() {
    if (m_aiHops != NULL) {
        delete[] m_aiHops;
    }
    if (m_adDist != NULL) {
        delete[] m_adDist;
    }
    if (m_adTime != NULL) {
        delete[] m_adTime;
    }
#ifdef OMP_A
	delete[] m_aStatLocks;	
#endif
}
