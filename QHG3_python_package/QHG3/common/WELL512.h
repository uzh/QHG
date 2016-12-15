#ifndef __WELL512_H__
#define __WELL512_H__

#include <stdlib.h>
#include <stdint.h>

static const unsigned int STATE_SIZE = 16;
static const unsigned long MAX_WRAND = 0xffffffff;

class WELL512 {
public:
    WELL512(uint32_t *aulState);
    WELL512();
   ~WELL512() { };


    uint32_t wrand();
    double wrandd() { return (1.0*wrand())/(1+MAX_WRAND); };
    double wrandr(double a, double b) { return a + ((b-a)*wrand())/(1+MAX_WRAND); };
    const uint32_t *getState() const { return m_aulState;};
private:
    uint32_t  m_aulState[STATE_SIZE];
    uint32_t  m_uiIndex;
};


#endif

