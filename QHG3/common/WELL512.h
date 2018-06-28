/*============================================================================
| WELL512
| 
|  Implementation of the WELL512 random number generation from a paper
|  by Chris Lomont: 
|    http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
 
#ifndef __WELL512_H__
#define __WELL512_H__

#include <stdlib.h>
#include <stdint.h>

static const unsigned int STATE_SIZE = 16;
static const unsigned long MAX_WRAND = 0xffffffff;

class WELL512 {
public:
    WELL512(uint32_t *aulState, uint32_t uiIndex=0);
    WELL512();
   ~WELL512() { };

    void seed(uint32_t *aulState, uint32_t uiIndex);

    // uint in [0, MAX_WRAND]
    uint32_t wrand();
    // double in [0,1)
    double   wrandd() { return (1.0*wrand())/(1+MAX_WRAND); };
    // double in [a,b)
    double   wrandr(double a, double b) { return a + ((b-a)*wrand())/(1+MAX_WRAND); };
    // uint in [a,b-1]
    uint32_t wrandi(uint32_t a, uint32_t b, uint32_t s=1) { uint32_t r=b-a; r+=(r%s); return a+s*(uint32_t)((1.0*r*wrand())/(s*(1.0+MAX_WRAND))); };
    
    // normal distribution around (0) with given sigma
    double   wgauss(double dSigma);

    const uint32_t *getState() const { return m_aulState;};
    const uint32_t  getIndex() const { return m_uiIndex;};
    unsigned long   getCount() { return m_iCount;};
private:
    uint32_t  m_aulState[STATE_SIZE];
    uint32_t  m_uiIndex;
    unsigned long  m_iCount;
    
    bool   m_bHavePrevNormal;
    double m_dPrevNormal;
};


#endif

