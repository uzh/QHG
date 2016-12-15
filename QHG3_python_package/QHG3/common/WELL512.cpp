#include <stdlib.h>

#include <time.h>
#include <omp.h>

#include "WELL512.h"

//-------------------------------------------------------------------------
// constructor 
//   specify state
//
WELL512::WELL512(unsigned int *aulState) 
    : m_uiIndex(0) {

    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        m_aulState[i] = aulState[i];
    }
}


//-------------------------------------------------------------------------
// constructor 
//   state "calculated" from current time
//
WELL512::WELL512() 
    : m_uiIndex(0) {

    // use number of seconds as seed
    time_t t = time(NULL);
    srand((uint)t);

    uint32_t *p = (uint32_t *)m_aulState;
    for (unsigned int i = 0; i < STATE_SIZE; i++) {
        *p++ = rand();
    }
}


//-------------------------------------------------------------------------
// wrand
//   next  random number
//   (from a paper by Chris Lomont)
//
uint32_t WELL512::wrand() {
    
    uint32_t a = m_aulState[m_uiIndex];
    uint32_t c = m_aulState[(m_uiIndex+13) & 15];
    uint32_t b = a ^ c ^ (a<<16) ^ (c<<15);
    c = m_aulState[(m_uiIndex+9) & 15];
    c ^= (c>>11);
    a = b ^c;
    m_aulState[m_uiIndex] = a;
    uint32_t d =  a ^ ((a<<5) & (uint32_t)0xDA442D24UL);
    m_uiIndex = (m_uiIndex+15) & 15;
    a = m_aulState[m_uiIndex];
    m_aulState[m_uiIndex] = a ^ b ^ d ^ (a<<2) ^ (b<<18) ^ (c << 28);

    return m_aulState[m_uiIndex];
}
    
