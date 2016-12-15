#ifndef __SMALLPRIMES_H__
#define __SMALLPRIMES_H__

#include <vector>

typedef std::vector<unsigned short> VECUS; 


int divideN(unsigned short *piN, unsigned short p);
void primiFactor(unsigned short iN, VECUS &vPrimes, VECUS &vPowers);

#endif

