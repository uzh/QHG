#include "smallprimes.h"

unsigned short s_aFirstPrimes[] = {
      2,   3,   5,   7,
     11,  13,  17,  19,
     23,  29,  31,  37,
     41,  43,  47,  53,
     59,  61,  67,  71,
     73,  79,  83,  89,
     97, 101, 103, 107,
    109, 113, 127, 131};

//-------------------------------------------------------------------------------
// divideN
//   Divide piN by p as often as possible.
//   Returns highest k such that p^k divides *piN
//   Value of *piN after NDivide is *piN/(p^k)
//
int divideN(unsigned short *piN, unsigned short p) {
    int i = 0;
    while ((*piN)%p == 0) {
        ++i;
        (*piN) /= p;
    }
    return i;
}

//-------------------------------------------------------------------------------
// primiFactor
//   Calculates prime factors of iN.
//   After PrimiFactor, vPrimes holds all primes in factorization of iN,
//   and vPowers holds the corresponding powers
//
void primiFactor(unsigned short iN, VECUS &vPrimes, VECUS &vPowers) {
    unsigned short p=2;
    unsigned short m=sizeof(s_aFirstPrimes)/sizeof(unsigned short); 
    for (int j = 0; (j < m); ++j) {
        p = s_aFirstPrimes[j];
        if (p*p <= iN) { 
	    unsigned short i = (unsigned short)divideN(&iN, p);
            if (i > 0) {
                vPrimes.push_back(p);
                vPowers.push_back(i);  
            }
        } else {
            break;
        }
    }
	
    p += 2;

    while (p*p <= iN) {
	unsigned short i = (unsigned short) divideN(&iN, p);
        if (i > 0) {
            vPrimes.push_back(p);
            vPowers.push_back(i);  
        }
        p+=2;
    }

    if (iN > 1) {
        vPrimes.push_back(iN);
        vPowers.push_back(1);
    }
}
